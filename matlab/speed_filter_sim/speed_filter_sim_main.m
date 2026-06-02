%% 速度估计结果滤波仿真
% 说明：
% 1. 这里不完整复现 FOC 和 STO+PLL，只模拟“估计速度带噪声后再滤波”的过程。
% 2. 目的是先验证滤波策略趋势，再和板上 CSV 实测结果对照。
% 3. 方法名称尽量和固件、CSV、HTML 页面保持一致，方便写报告。

clear; clc; close all;

outDir = fullfile(pwd, 'sim_outputs');
if ~exist(outDir, 'dir')
    mkdir(outDir);
end

Ts = 0.02;                 % 采样周期，约等于 20 ms 一次记录
tEnd = 24.0;
t = (0:Ts:tEnd)';

rng(20260530);             % 固定随机种子，保证每次仿真可复现

scenes = {
    makeScene('step_0_500', t, [0 1.0 18.0 20.0], [0 500 500 0])
    makeScene('profile_0_500_1000_500', t, [0 1.0 7.0 13.0 19.0 21.0], [0 500 1000 500 0 0])
};

for sceneIndex = 1:numel(scenes)
    scene = scenes{sceneIndex};

    trueSpeed = simulateMotorSpeed(scene.target, Ts);
    rawSpeed = simulateEstimatedSpeed(trueSpeed, scene.target, t);

    results = {};
    results{end + 1} = applyFilter('NONE', rawSpeed, scene.target, Ts);
    results{end + 1} = applyFilter('LPF1_shift2', rawSpeed, scene.target, Ts);
    results{end + 1} = applyFilter('LPF1_shift3', rawSpeed, scene.target, Ts);
    results{end + 1} = applyFilter('LPF1_shift4', rawSpeed, scene.target, Ts);
    results{end + 1} = applyFilter('MOVAVG8', rawSpeed, scene.target, Ts);
    results{end + 1} = applyFilter('WMA4', rawSpeed, scene.target, Ts);
    results{end + 1} = applyFilter('ADALPF_2to4_h18_40_n32', rawSpeed, scene.target, Ts);

    drawScene(outDir, scene.name, t, scene.target, rawSpeed, trueSpeed, results);
    drawSingleMethodScenes(outDir, scene.name, t, scene.target, rawSpeed, trueSpeed, results);
    writeMetrics(outDir, scene.name, t, scene.target, results);
end

disp('仿真完成，结果保存在 sim_outputs 目录。');

function scene = makeScene(name, t, keyTime, keySpeed)
    target = interp1(keyTime(:), keySpeed(:), t, 'previous', 'extrap');
    scene.name = name;
    scene.target = target;
end

function y = simulateMotorSpeed(target, Ts)
    % 用一阶惯性近似电机转速跟随。加速和减速使用不同时间常数，模拟实际系统不完全对称。
    y = zeros(size(target));
    tauUp = 0.34;
    tauDown = 0.24;

    for k = 2:numel(target)
        if target(k) >= y(k - 1)
            tau = tauUp;
        else
            tau = tauDown;
        end
        alpha = Ts / (tau + Ts);
        y(k) = y(k - 1) + alpha * (target(k) - y(k - 1));
    end
end

function raw = simulateEstimatedSpeed(trueSpeed, target, t)
    % 估计速度 = 真实速度 + 随机噪声 + 周期纹波 + 动态扰动。
    % 这只是用于滤波验证的简化模型，不代表完整 STO+PLL 数学模型。
    n = numel(trueSpeed);
    noise = 10.0 * randn(n, 1);
    ripple = 14.0 * sin(2 * pi * 7.0 * t) + 6.0 * sin(2 * pi * 17.0 * t);

    dTarget = [0; diff(target)];
    dynamicKick = zeros(n, 1);
    eventIndex = find(abs(dTarget) > 1);
    for i = 1:numel(eventIndex)
        idx = eventIndex(i);
        kickSign = sign(dTarget(idx));
        span = idx:min(n, idx + round(0.5 / (t(2) - t(1))));
        decayT = (0:numel(span)-1)' * (t(2) - t(1));
        dynamicKick(span) = dynamicKick(span) + kickSign * 55.0 * exp(-decayT / 0.18);
    end

    lowSpeedPenalty = 18.0 * exp(-abs(trueSpeed) / 120.0) .* randn(n, 1);
    raw = trueSpeed + noise + ripple + dynamicKick + lowSpeedPenalty;
end

function result = applyFilter(name, raw, target, Ts)
    y = zeros(size(raw));

    switch name
        case 'NONE'
            y = raw;

        case 'LPF1_shift2'
            y = lpfShift(raw, 2);

        case 'LPF1_shift3'
            y = lpfShift(raw, 3);

        case 'LPF1_shift4'
            y = lpfShift(raw, 4);

        case 'MOVAVG8'
            y = movingAverage(raw, 8);

        case 'WMA4'
            y = weightedMovingAverage(raw, [1 2 3 4]);

        case 'ADALPF_2to4_h18_40_n32'
            y = adaptiveLpf(raw, target, 2, 4, 18, 40, 32);

        otherwise
            error('未知滤波方法：%s', name);
    end

    result.name = name;
    result.y = y;
    result.Ts = Ts;
end

function y = lpfShift(x, shift)
    y = zeros(size(x));
    y(1) = x(1);
    gain = 1 / (2 ^ shift);
    for k = 2:numel(x)
        y(k) = y(k - 1) + gain * (x(k) - y(k - 1));
    end
end

function y = movingAverage(x, n)
    y = zeros(size(x));
    for k = 1:numel(x)
        left = max(1, k - n + 1);
        y(k) = mean(x(left:k));
    end
end

function y = weightedMovingAverage(x, weights)
    y = zeros(size(x));
    weights = weights(:);
    n = numel(weights);
    for k = 1:numel(x)
        left = max(1, k - n + 1);
        window = x(left:k);
        useWeights = weights(end - numel(window) + 1:end);
        y(k) = sum(window(:) .* useWeights) / sum(useWeights);
    end
end

function y = adaptiveLpf(raw, target, fastShift, slowShift, enterRpm, exitRpm, confirmN)
    y = zeros(size(raw));
    y(1) = raw(1);
    currentShift = fastShift;
    steadyCounter = 0;

    for k = 2:numel(raw)
        err = abs(target(k) - raw(k));

        if err >= exitRpm
            steadyCounter = 0;
            activeShift = fastShift;
        elseif err <= enterRpm
            steadyCounter = min(confirmN, steadyCounter + 1);
            if steadyCounter >= confirmN
                activeShift = slowShift;
            else
                activeShift = currentShift;
            end
        else
            activeShift = currentShift;
        end

        currentShift = activeShift;
        gain = 1 / (2 ^ activeShift);
        y(k) = y(k - 1) + gain * (raw(k) - y(k - 1));
    end
end

function drawScene(outDir, sceneName, t, target, raw, trueSpeed, results)
    fig = figure('Visible', 'off', 'Color', 'w', 'Position', [80 80 1280 720]);
    hold on; grid on;

    plot(t, target, 'k--', 'LineWidth', 1.8, 'DisplayName', 'Target speed');
    plot(t, trueSpeed, 'Color', [0.20 0.20 0.20], 'LineWidth', 1.2, 'DisplayName', 'Simulated true speed');
    plot(t, raw, 'Color', [0.75 0.75 0.75], 'LineWidth', 0.8, 'DisplayName', 'Raw estimated speed');

    colors = lines(numel(results));
    for i = 1:numel(results)
        plot(t, results{i}.y, 'LineWidth', 1.4, 'Color', colors(i, :), 'DisplayName', results{i}.name);
    end

    xlabel('Time / s');
    ylabel('Speed / rpm');
    title(['Speed Filter Simulation: ', sceneName], 'Interpreter', 'none');
    lgd = legend('show');
    set(lgd, 'Location', 'bestoutside');
    set(lgd, 'Interpreter', 'none');

    saveas(fig, fullfile(outDir, [sceneName, '_filter_compare.png']));
    close(fig);
end

function drawSingleMethodScenes(outDir, sceneName, t, target, raw, trueSpeed, results)
    for i = 1:numel(results)
        fig = figure('Visible', 'off', 'Color', 'w', 'Position', [80 80 1280 720]);
        hold on; grid on;

        plot(t, target, 'k--', 'LineWidth', 1.8, 'DisplayName', 'Target speed');
        plot(t, trueSpeed, 'Color', [0.25 0.25 0.25], 'LineWidth', 1.2, 'DisplayName', 'Simulated true speed');
        plot(t, raw, 'Color', [0.80 0.80 0.80], 'LineWidth', 0.8, 'DisplayName', 'Raw estimated speed');
        plot(t, results{i}.y, 'Color', [0.80 0.12 0.18], 'LineWidth', 1.8, 'DisplayName', results{i}.name);

        xlabel('Time / s');
        ylabel('Speed / rpm');
        title(['Single Filter View: ', sceneName, ' | ', results{i}.name], 'Interpreter', 'none');

        lgd = legend('show');
        set(lgd, 'Location', 'bestoutside');
        set(lgd, 'Interpreter', 'none');

        saveas(fig, fullfile(outDir, [sceneName, '_single_', results{i}.name, '.png']));
        close(fig);
    end
end

function writeMetrics(outDir, sceneName, t, target, results)
    rows = cell(numel(results) + 1, 6);
    rows(1, :) = {'method', 'peak_rpm', 'overshoot_rpm', 'steady_mean_rpm', 'steady_std_rpm', 'settle_time_s'};

    finalTarget = max(target);
    steadyMask = t >= 5.0 & t <= 6.8 & target == 500;
    if strcmp(sceneName, 'profile_0_500_1000_500')
        steadyMask = t >= 14.5 & t <= 18.5 & target == 500;
    end

    for i = 1:numel(results)
        y = results{i}.y;
        peak = max(y);
        overshoot = peak - finalTarget;
        steadyMean = mean(y(steadyMask));
        steadyStd = std(y(steadyMask));
        settleTime = calcSettleTime(t, target, y);

        rows(i + 1, :) = {results{i}.name, peak, overshoot, steadyMean, steadyStd, settleTime};
    end

    writeCellCsv(fullfile(outDir, [sceneName, '_metrics.csv']), rows);
end

function settleTime = calcSettleTime(t, target, y)
    targetValue = 500;
    startIndex = find(target >= targetValue, 1, 'first');
    if isempty(startIndex)
        settleTime = NaN;
        return;
    end

    tolerance = 0.05 * targetValue;
    settleTime = NaN;
    for k = startIndex:numel(y)
        tailIndex = k:min(numel(y), k + round(0.5 / (t(2) - t(1))));
        if all(abs(y(tailIndex) - targetValue) <= tolerance)
            settleTime = t(k) - t(startIndex);
            return;
        end
    end
end

function writeCellCsv(pathName, rows)
    fid = fopen(pathName, 'w');
    if fid < 0
        error('无法写入文件：%s', pathName);
    end

    cleanup = onCleanup(@() fclose(fid));
    for r = 1:size(rows, 1)
        lineParts = cell(1, size(rows, 2));
        for c = 1:size(rows, 2)
            value = rows{r, c};
            if isnumeric(value)
                lineParts{c} = sprintf('%.6g', value);
            else
                lineParts{c} = char(value);
            end
        end
        fprintf(fid, '%s\n', strjoin(lineParts, ','));
    end
end
