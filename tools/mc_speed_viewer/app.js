const state = {
  datasets: [],
  columns: [],
  analysisMode: "filter",
  xColumn: "",
  yColumn: "",
  y2Column: "",
  compareColumn: "",
  layoutMode: "overlay",
  normalizeTime: true,
  showPoints: false,
  showLegend: true,
  showY2: false,
  groupByColumn: true,
  ranges: {
    xMin: null,
    xMax: null,
    yMin: null,
    yMax: null
  }
};

const palette = [
  "#ff5d73",
  "#00a6fb",
  "#06d6a0",
  "#ff9f1c",
  "#8b5cf6",
  "#ef476f",
  "#118ab2",
  "#3a86ff",
  "#fb5607",
  "#2ec4b6"
];

const targetLineColor = "#374151";

const dom = {
  csvInput: document.getElementById("csvInput"),
  dropZone: document.getElementById("dropZone"),
  clearAllBtn: document.getElementById("clearAllBtn"),
  analysisModeSelect: document.getElementById("analysisModeSelect"),
  modeGuideTitle: document.getElementById("modeGuideTitle"),
  modeGuideText: document.getElementById("modeGuideText"),
  presetRawFilteredBtn: document.getElementById("presetRawFilteredBtn"),
  presetFilteredTargetBtn: document.getElementById("presetFilteredTargetBtn"),
  presetStartupBtn: document.getElementById("presetStartupBtn"),
  presetSteadyBtn: document.getElementById("presetSteadyBtn"),
  xColumnSelect: document.getElementById("xColumnSelect"),
  yColumnSelect: document.getElementById("yColumnSelect"),
  y2ColumnSelect: document.getElementById("y2ColumnSelect"),
  compareColumnSelect: document.getElementById("compareColumnSelect"),
  layoutModeSelect: document.getElementById("layoutModeSelect"),
  normalizeTimeCheckbox: document.getElementById("normalizeTimeCheckbox"),
  showPointsCheckbox: document.getElementById("showPointsCheckbox"),
  showLegendCheckbox: document.getElementById("showLegendCheckbox"),
  showY2Checkbox: document.getElementById("showY2Checkbox"),
  groupByColumnCheckbox: document.getElementById("groupByColumnCheckbox"),
  xMinInput: document.getElementById("xMinInput"),
  xMaxInput: document.getElementById("xMaxInput"),
  yMinInput: document.getElementById("yMinInput"),
  yMaxInput: document.getElementById("yMaxInput"),
  applyRangeBtn: document.getElementById("applyRangeBtn"),
  resetRangeBtn: document.getElementById("resetRangeBtn"),
  datasetList: document.getElementById("datasetList"),
  seriesList: document.getElementById("seriesList"),
  datasetCount: document.getElementById("datasetCount"),
  pointCount: document.getElementById("pointCount"),
  currentYLabel: document.getElementById("currentYLabel"),
  windowLabel: document.getElementById("windowLabel"),
  statsTableBody: document.querySelector("#statsTable tbody"),
  experimentTableHeadRow: document.getElementById("experimentTableHeadRow"),
  experimentTableBody: document.querySelector("#experimentTable tbody"),
  experimentMetricNote: document.getElementById("experimentMetricNote"),
  exportPngBtn: document.getElementById("exportPngBtn"),
  canvas: document.getElementById("chartCanvas"),
  smallMultiplesGrid: document.getElementById("smallMultiplesGrid"),
  legendExplainList: document.getElementById("legendExplainList")
};

const methodLabelMap = {
  NONE: "NONE：无滤波",
  LPF1: "LPF1：一阶低通",
  MOVAVG8: "MOVAVG8：8点滑动平均",
  WMA4: "WMA4：4点加权平均",
  ADALPF: "ADALPF：自适应低通",
  PLL_FIX: "PLL_FIX：固定参数锁相环",
  PLL_SPLIT: "PLL_SPLIT：分程参数锁相环"
};

const shortMethodLabelMap = {
  NONE: "无滤波",
  LPF1: "一阶低通",
  MOVAVG8: "滑动平均",
  WMA4: "加权平均",
  ADALPF: "自适应低通",
  PLL_FIX: "固定PLL",
  PLL_SPLIT: "分程PLL"
};

const analysisModeMap = {
  filter: {
    title: "滤波方法对比",
    description:
      "重点看原始速度、滤波速度和目标速度之间的差别，适合展示不同滤波方法在启动、调速和稳态阶段的效果。"
  },
  pll: {
    title: "PLL PI 参数对比",
    description:
      "重点看不同 PLL 参数组合下原始速度估计的动态差异。PLL 的 Kp、Ki 直接作用于估计器内部锁相环，因此这里默认比较原始速度与目标转速，而不混入控制使用速度。"
  },
  switch: {
    title: "切换保护对比",
    description:
      "重点看启动到闭环切换附近的平顺性，观察是否存在切换顿挫、速度突跳、反向毛刺，以及保护处理是否有效。"
  }
};

const ctx = dom.canvas.getContext("2d");

bindEvents();
renderAll();

function bindEvents() {
  dom.csvInput.addEventListener("change", handleFileInput);
  dom.clearAllBtn.addEventListener("click", clearAll);
  dom.presetRawFilteredBtn.addEventListener("click", applyPresetRawFiltered);
  dom.presetFilteredTargetBtn.addEventListener("click", applyPresetFilteredTarget);
  dom.presetStartupBtn.addEventListener("click", applyPresetStartup);
  dom.presetSteadyBtn.addEventListener("click", applyPresetSteadyState);
  dom.analysisModeSelect.addEventListener("change", () => {
    state.analysisMode = dom.analysisModeSelect.value;
    renderModeGuide();
    applyModeDefaults();
    renderAll();
  });

  dom.xColumnSelect.addEventListener("change", () => {
    state.xColumn = dom.xColumnSelect.value;
    renderAll();
  });
  dom.yColumnSelect.addEventListener("change", () => {
    state.yColumn = dom.yColumnSelect.value;
    if (state.compareColumn === state.yColumn) {
      state.compareColumn = "";
    }
    renderAll();
  });
  dom.y2ColumnSelect.addEventListener("change", () => {
    state.y2Column = dom.y2ColumnSelect.value;
    renderAll();
  });
  dom.compareColumnSelect.addEventListener("change", () => {
    state.compareColumn = dom.compareColumnSelect.value;
    renderAll();
  });
  dom.layoutModeSelect.addEventListener("change", () => {
    state.layoutMode = dom.layoutModeSelect.value;
    renderAll();
  });
  dom.normalizeTimeCheckbox.addEventListener("change", () => {
    state.normalizeTime = dom.normalizeTimeCheckbox.checked;
    renderAll();
  });
  dom.showPointsCheckbox.addEventListener("change", () => {
    state.showPoints = dom.showPointsCheckbox.checked;
    renderAll();
  });
  dom.showLegendCheckbox.addEventListener("change", () => {
    state.showLegend = dom.showLegendCheckbox.checked;
    renderAll();
  });
  dom.showY2Checkbox.addEventListener("change", () => {
    state.showY2 = dom.showY2Checkbox.checked;
    renderAll();
  });
  dom.groupByColumnCheckbox.addEventListener("change", () => {
    state.groupByColumn = dom.groupByColumnCheckbox.checked;
    assignDatasetColors();
    renderAll();
  });
  dom.applyRangeBtn.addEventListener("click", applyRangesFromInputs);
  dom.resetRangeBtn.addEventListener("click", resetRanges);
  dom.exportPngBtn.addEventListener("click", exportPng);

  ["dragenter", "dragover"].forEach((eventName) => {
    dom.dropZone.addEventListener(eventName, (event) => {
      event.preventDefault();
      dom.dropZone.classList.add("drag-over");
    });
  });

  ["dragleave", "drop"].forEach((eventName) => {
    dom.dropZone.addEventListener(eventName, (event) => {
      event.preventDefault();
      dom.dropZone.classList.remove("drag-over");
    });
  });

  dom.dropZone.addEventListener("drop", async (event) => {
    const files = Array.from(event.dataTransfer?.files || []).filter((file) =>
      file.name.toLowerCase().endsWith(".csv")
    );
    if (files.length) {
      await loadFiles(files);
    }
  });
}

async function handleFileInput(event) {
  const files = Array.from(event.target.files || []);
  if (!files.length) {
    return;
  }
  await loadFiles(files);
  dom.csvInput.value = "";
}

async function loadFiles(files) {
  const parsedDatasets = [];
  for (const [index, file] of files.entries()) {
    const text = await file.text();
    const dataset = parseCsvText(text, file.name, state.datasets.length + index);
    if (dataset) {
      parsedDatasets.push(dataset);
    }
  }

  if (!parsedDatasets.length) {
    return;
  }

  state.datasets.push(...parsedDatasets);
  refreshColumnsFromDatasets();
  assignDatasetColors();
  renderAll();
}

function parseCsvText(text, name, order) {
  const lines = text
    .replace(/^\uFEFF/, "")
    .split(/\r?\n/)
    .filter((line) => line.trim().length > 0);

  if (lines.length < 2) {
    return null;
  }

  const headers = splitCsvLine(lines[0]).map((item, idx) => {
    const value = item.trim();
    return value || `col_${idx + 1}`;
  });

  const rows = [];
  for (let i = 1; i < lines.length; i += 1) {
    const values = splitCsvLine(lines[i]);
    const row = {};
    headers.forEach((header, idx) => {
      const raw = (values[idx] || "").trim();
      const numeric = Number(raw);
      row[header] = Number.isFinite(numeric) ? numeric : raw;
    });
    rows.push(row);
  }

  return {
    id: `${name}-${Date.now()}-${order}`,
    name,
    rows,
    headers,
    color: palette[order % palette.length],
    visible: true,
    note: ""
  };
}

function splitCsvLine(line) {
  const result = [];
  let current = "";
  let inQuotes = false;

  for (let i = 0; i < line.length; i += 1) {
    const char = line[i];
    const next = line[i + 1];

    if (char === '"' && inQuotes && next === '"') {
      current += '"';
      i += 1;
      continue;
    }

    if (char === '"') {
      inQuotes = !inQuotes;
      continue;
    }

    if (char === "," && !inQuotes) {
      result.push(current);
      current = "";
      continue;
    }

    current += char;
  }

  result.push(current);
  return result;
}

function refreshColumnsFromDatasets() {
  const columnSet = new Set();
  state.datasets.forEach((dataset) => {
    dataset.headers.forEach((header) => columnSet.add(header));
  });
  state.columns = Array.from(columnSet);

  if (!state.columns.length) {
    state.xColumn = "";
    state.yColumn = "";
    state.y2Column = "";
    state.compareColumn = "";
    return;
  }

  const findCandidate = (keywords, exclude = []) => {
    const lowerColumns = state.columns.map((item) => item.toLowerCase());
    const index = lowerColumns.findIndex((column, idx) => {
      if (exclude.includes(state.columns[idx])) {
        return false;
      }
      return keywords.some((keyword) => column.includes(keyword));
    });
    return index >= 0 ? state.columns[index] : "";
  };

  if (!state.xColumn || !state.columns.includes(state.xColumn)) {
    state.xColumn = findCandidate(["time", "timestamp", "ms", "sample"]) || state.columns[0];
  }

  if (!state.yColumn || !state.columns.includes(state.yColumn)) {
    state.yColumn =
      findCandidate(["filtered", "speed", "rpm", "velocity"]) ||
      state.columns[Math.min(1, state.columns.length - 1)];
  }

  if (!state.compareColumn || !state.columns.includes(state.compareColumn) || state.compareColumn === state.yColumn) {
    state.compareColumn =
      findCandidate(["raw", "target", "ref"], [state.yColumn]) ||
      "";
  }

  if (!state.y2Column || !state.columns.includes(state.y2Column)) {
    state.y2Column =
      findCandidate(["target", "ref"], [state.yColumn, state.compareColumn]) ||
      "";
  }
}

function assignDatasetColors() {
  state.datasets.forEach((dataset, index) => {
    dataset.color = palette[index % palette.length];
  });
}

function renderAll() {
  renderModeGuide();
  renderPresetButtonLabels();
  renderColumnSelectors();
  renderDatasetList();
  renderSeriesList();
  renderSummary();
  renderStatsTable();
  renderExperimentTableHeader();
  renderExperimentTable();
  renderExperimentMetricNote();
  renderChart();
  renderLegendExplainBox();
}

function renderModeGuide() {
  const modeMeta = analysisModeMap[state.analysisMode] || analysisModeMap.filter;
  dom.analysisModeSelect.value = state.analysisMode;
  dom.modeGuideTitle.textContent = modeMeta.title;
  dom.modeGuideText.textContent = modeMeta.description;
}

function renderPresetButtonLabels() {
  if (state.analysisMode === "pll") {
    dom.presetRawFilteredBtn.textContent = "参数 vs 目标";
    dom.presetFilteredTargetBtn.textContent = "PLL 内部量";
    dom.presetStartupBtn.textContent = "启动切入";
    dom.presetSteadyBtn.textContent = "稳态调速";
    return;
  }

  if (state.analysisMode === "switch") {
    dom.presetRawFilteredBtn.textContent = "切换前后";
    dom.presetFilteredTargetBtn.textContent = "最终速度 vs 目标";
    dom.presetStartupBtn.textContent = "切换放大";
    dom.presetSteadyBtn.textContent = "切换后稳态";
    return;
  }

  dom.presetRawFilteredBtn.textContent = "原始 vs 滤波";
  dom.presetFilteredTargetBtn.textContent = "滤波 vs 目标";
  dom.presetStartupBtn.textContent = "启动放大";
  dom.presetSteadyBtn.textContent = "稳态波动";
}

function renderColumnSelectors() {
  renderSelect(dom.xColumnSelect, state.columns, state.xColumn, true);
  renderSelect(dom.yColumnSelect, state.columns, state.yColumn, true);
  renderSelect(dom.y2ColumnSelect, state.columns, state.y2Column, false);
  renderSelect(dom.compareColumnSelect, state.columns, state.compareColumn, false);

  dom.normalizeTimeCheckbox.checked = state.normalizeTime;
  dom.layoutModeSelect.value = state.layoutMode;
  dom.showPointsCheckbox.checked = state.showPoints;
  dom.showLegendCheckbox.checked = state.showLegend;
  dom.showY2Checkbox.checked = state.showY2;
  dom.groupByColumnCheckbox.checked = state.groupByColumn;
}

function renderSelect(selectEl, options, currentValue, required) {
  const previous = currentValue;
  selectEl.innerHTML = "";
  if (!required) {
    const empty = document.createElement("option");
    empty.value = "";
    empty.textContent = "不使用";
    selectEl.appendChild(empty);
  }

  options.forEach((option) => {
    const el = document.createElement("option");
    el.value = option;
    el.textContent = option;
    if (option === previous) {
      el.selected = true;
    }
    selectEl.appendChild(el);
  });
}

function renderDatasetList() {
  if (!state.datasets.length) {
    dom.datasetList.className = "dataset-list empty-state";
    dom.datasetList.textContent = "还没有导入 CSV 文件";
    return;
  }

  dom.datasetList.className = "dataset-list";
  dom.datasetList.innerHTML = "";

  state.datasets.forEach((dataset) => {
    const item = document.createElement("div");
    item.className = "dataset-item";

    const header = document.createElement("header");
    const title = document.createElement("strong");
    title.textContent = buildShortDatasetLabel(dataset);

    const toggleWrap = document.createElement("label");
    toggleWrap.className = "dataset-toggle";
    const checkbox = document.createElement("input");
    checkbox.type = "checkbox";
    checkbox.checked = dataset.visible;
    checkbox.addEventListener("change", () => {
      dataset.visible = checkbox.checked;
      renderAll();
    });
    const swatch = document.createElement("span");
    swatch.className = "swatch";
    swatch.style.background = dataset.color;
    toggleWrap.append(checkbox, swatch);

    header.append(title, toggleWrap);

    const meta = document.createElement("div");
    meta.className = "dataset-meta";
    const datasetMeta = extractDatasetMeta(dataset);
    meta.innerHTML = [
      `完整说明：${escapeHtml(buildFullDatasetLabel(dataset))}`,
      `原始文件：${escapeHtml(dataset.name)}`,
      `方法说明：${escapeHtml(datasetMeta.methodName)}`,
      `列数：${dataset.headers.length}`,
      `采样点：${dataset.rows.length}`,
      `主要列：${dataset.headers.slice(0, 5).join(" / ")}`
    ].join("<br>");

    const noteInput = document.createElement("input");
    noteInput.type = "text";
    noteInput.className = "dataset-note-input";
    noteInput.placeholder = "备注：例如 LPF shift=3 / MovingAvg N=8";
    noteInput.value = dataset.note;
    noteInput.addEventListener("input", () => {
      dataset.note = noteInput.value;
      renderSeriesList();
    });

    item.append(header, meta, noteInput);
    dom.datasetList.appendChild(item);
  });
}

function renderSeriesList() {
  const seriesList = collectSeries({ includeHiddenAxis: true });
  if (!seriesList.length) {
    dom.seriesList.className = "series-list empty-state";
    dom.seriesList.textContent = "导入 CSV 后会在这里显示每条曲线";
    return;
  }

  dom.seriesList.className = "series-list";
  dom.seriesList.innerHTML = "";

  seriesList.forEach((series) => {
    const item = document.createElement("div");
    item.className = "series-item";

    const top = document.createElement("div");
    top.className = "series-top";

    const title = document.createElement("div");
    title.className = "series-title";
    title.textContent = `${series.datasetLabel} · ${translateColumnLabel(series.column)}`;

    const meta = document.createElement("div");
    meta.className = "series-axis";
    meta.textContent = series.axis === "y2" ? "第二 Y 轴" : "主 Y 轴";

    const swatch = document.createElement("span");
    swatch.className = "swatch";
    swatch.style.background = series.color;

    top.append(swatch, title, meta);

    const detail = document.createElement("div");
    detail.className = "series-detail";
    detail.textContent = series.datasetNote
      ? `备注：${series.datasetNote}`
      : `来源：${series.datasetName}`;

    item.append(top, detail);
    dom.seriesList.appendChild(item);
  });
}

function renderSummary() {
  const visibleDatasets = state.datasets.filter((dataset) => dataset.visible);
  const pointCount = visibleDatasets.reduce((sum, dataset) => sum + dataset.rows.length, 0);

  dom.datasetCount.textContent = String(visibleDatasets.length);
  dom.pointCount.textContent = String(pointCount);

  const labels = [state.yColumn, state.compareColumn, state.showY2 ? state.y2Column : ""].filter(Boolean);
  dom.currentYLabel.textContent = labels.length ? labels.join(" + ") : "未选择";

  const hasCustomRange = Object.values(state.ranges).some((value) => value !== null);
  dom.windowLabel.textContent = hasCustomRange ? "局部窗口" : "全局";
}

function applyRangesFromInputs() {
  state.ranges.xMin = parseOptionalNumber(dom.xMinInput.value);
  state.ranges.xMax = parseOptionalNumber(dom.xMaxInput.value);
  state.ranges.yMin = parseOptionalNumber(dom.yMinInput.value);
  state.ranges.yMax = parseOptionalNumber(dom.yMaxInput.value);
  renderAll();
}

function resetRanges() {
  state.ranges = {
    xMin: null,
    xMax: null,
    yMin: null,
    yMax: null
  };
  dom.xMinInput.value = "";
  dom.xMaxInput.value = "";
  dom.yMinInput.value = "";
  dom.yMaxInput.value = "";
  renderAll();
}

function parseOptionalNumber(value) {
  if (value === "" || value == null) {
    return null;
  }
  const number = Number(value);
  return Number.isFinite(number) ? number : null;
}

function collectSeries(options = {}) {
  const includeHiddenAxis = Boolean(options.includeHiddenAxis);
  if (!state.xColumn || !state.yColumn) {
    return [];
  }

  const specs = [
    { column: state.yColumn, axis: "y", type: "primary" },
    ...(state.compareColumn ? [{ column: state.compareColumn, axis: "y", type: "compare" }] : []),
    ...((includeHiddenAxis || state.showY2) && state.y2Column
      ? [{ column: state.y2Column, axis: "y2", type: "secondary" }]
      : [])
  ];

  return state.datasets
    .filter((dataset) => dataset.visible)
    .flatMap((dataset, datasetIndex) =>
      specs
        .filter((spec, specIndex, arr) => arr.findIndex((item) => item.column === spec.column && item.axis === spec.axis) === specIndex)
        .map((spec) => buildSeriesFromDataset(dataset, datasetIndex, spec))
        .filter(Boolean)
    );
}

function buildSeriesFromDataset(dataset, datasetIndex, spec) {
  if (!dataset.headers.includes(state.xColumn) || !dataset.headers.includes(spec.column)) {
    return null;
  }

  const phaseColumn = dataset.headers.includes("phase") ? "phase" : "";

  const rows = dataset.rows
    .map((row) => {
      const x = row[state.xColumn];
      const y = row[spec.column];
      if (!isFiniteNumber(x) || !isFiniteNumber(y)) {
        return null;
      }
      return {
        x,
        y,
        phase: phaseColumn ? String(row[phaseColumn] || "") : "",
        breakAfter: false
      };
    })
    .filter(Boolean);

  if (!rows.length) {
    return null;
  }

  if (state.normalizeTime) {
    const offset = rows[0].x;
    rows.forEach((point) => {
      point.x -= offset;
    });
  }

  markDiscontinuities(rows, spec.column);

  return {
    id: `${dataset.id}-${spec.axis}-${spec.column}`,
    datasetId: dataset.id,
    datasetName: dataset.name,
    datasetLabel: buildShortDatasetLabel(dataset),
    datasetNote: dataset.note,
    datasetIndex,
    column: spec.column,
    axis: spec.axis,
    type: spec.type,
    rows,
    color: pickSeriesColor(datasetIndex, spec),
    lineDash: pickSeriesLineDash(spec)
  };
}

function pickSeriesColor(datasetIndex, spec) {
  if (spec.axis === "y2") {
    return targetLineColor;
  }

  if (spec.type === "compare") {
    return palette[(datasetIndex + 4) % palette.length];
  }

  return palette[datasetIndex % palette.length];
}

function pickSeriesLineDash(spec) {
  if (spec.axis === "y2") {
    return [10, 8];
  }

  const column = String(spec.column || "").toLowerCase();

  if (state.analysisMode === "pll") {
    if (column.includes("raw") && column.includes("speed")) {
      return [];
    }
    if (column.includes("filtered") && column.includes("speed")) {
      return [7, 5];
    }
    if (column.includes("final") && column.includes("speed")) {
      return [7, 5];
    }
  }

  if (column.includes("filtered") && column.includes("speed")) {
    return [];
  }

  if (column.includes("raw") && column.includes("speed")) {
    return [8, 6];
  }

  if (spec.type === "compare") {
    return [7, 5];
  }

  return [];
}

function markDiscontinuities(rows, columnName) {
  const column = String(columnName || "").toLowerCase();
  const isSpeedColumn = column.includes("speed");
  if (!isSpeedColumn || rows.length < 2) {
    return;
  }

  for (let i = 0; i < rows.length - 1; i += 1) {
    const current = rows[i];
    const next = rows[i + 1];
    const phaseChanged = current.phase && next.phase && current.phase !== next.phase;
    const droppedToZero = Math.abs(next.y) < 1 && Math.abs(current.y) > 80;
    const abruptJump = Math.abs(next.y - current.y) > 400;
    const tinyTimeGap = Math.abs(next.x - current.x) <= 30;

    if (tinyTimeGap && (phaseChanged && droppedToZero || abruptJump && droppedToZero)) {
      current.breakAfter = true;
    }
  }
}

function renderStatsTable() {
  const seriesList = collectSeries();
  dom.statsTableBody.innerHTML = "";

  seriesList.forEach((series) => {
    const points = filterPointsByWindow(series.rows);
    const stats = computeStats(points.map((point) => point.y));
    const tr = document.createElement("tr");
    tr.innerHTML = `
      <td>${escapeHtml(series.datasetLabel)}</td>
      <td>${escapeHtml(translateColumnLabel(series.column))}</td>
      <td>${series.axis === "y2" ? "Y2" : "Y1"}</td>
      <td>${points.length}</td>
      <td>${formatNumber(stats.mean)}</td>
      <td>${formatNumber(stats.min)}</td>
      <td>${formatNumber(stats.max)}</td>
      <td>${formatNumber(stats.peakToPeak)}</td>
      <td>${formatNumber(stats.std)}</td>
    `;
    dom.statsTableBody.appendChild(tr);
  });
}

function renderExperimentTable() {
  dom.experimentTableBody.innerHTML = "";

  if (state.analysisMode === "pll") {
    renderPllExperimentTable();
    return;
  }

  if (state.analysisMode === "switch") {
    renderSwitchExperimentTable();
    return;
  }

  const summaries = state.datasets
    .filter((dataset) => dataset.visible)
    .map((dataset) => buildExperimentSummary(dataset))
    .filter(Boolean);

  if (!summaries.length) {
    const tr = document.createElement("tr");
    tr.innerHTML = '<td colspan="12">导入包含时间、目标速度、滤波速度的 CSV 后，这里会自动生成实验指标。</td>';
    dom.experimentTableBody.appendChild(tr);
    return;
  }

  summaries.forEach((summary) => {
    const tr = document.createElement("tr");
    tr.innerHTML = `
      <td>${escapeHtml(summary.datasetLabel)}</td>
      <td>${escapeHtml(summary.methodName)}</td>
      <td>${escapeHtml(summary.paramTag)}</td>
      <td>${formatNumber(summary.targetSpeed)}</td>
      <td>${formatNumber(summary.riseTimeMs)}</td>
      <td>${formatNumber(summary.settlingTimeMs)}</td>
      <td>${formatNumber(summary.steadyMean)}</td>
      <td>${formatNumber(summary.steadyError)}</td>
      <td>${formatNumber(summary.steadyStd)}</td>
      <td>${formatNumber(summary.steadyPeakToPeak)}</td>
      <td>${formatNumber(summary.overshoot)}</td>
      <td>${formatNumber(summary.totalDurationMs)}</td>
    `;
    dom.experimentTableBody.appendChild(tr);
  });
}

function renderExperimentTableHeader() {
  if (state.analysisMode === "pll") {
    dom.experimentTableHeadRow.innerHTML = `
      <th>文件</th>
      <th>方法</th>
      <th>参数</th>
      <th>目标转速</th>
      <th>启动最小速度</th>
      <th>首次稳定为正(ms)</th>
      <th>切换突跳</th>
      <th>切换恢复(ms)</th>
      <th>稳态原始速度标准差</th>
      <th>稳态原始速度峰峰值</th>
      <th>调速段原始速度标准差</th>
      <th>是否有反向毛刺</th>
    `;
    return;
  }

  if (state.analysisMode === "switch") {
    dom.experimentTableHeadRow.innerHTML = `
      <th>文件</th>
      <th>版本</th>
      <th>方法</th>
      <th>参数</th>
      <th>切换时刻(ms)</th>
      <th>切换前速度</th>
      <th>切换后速度</th>
      <th>切换突跳</th>
      <th>恢复到450rpm(ms)</th>
      <th>启动最小速度</th>
      <th>是否有反向毛刺</th>
      <th>备注</th>
    `;
    return;
  }

  dom.experimentTableHeadRow.innerHTML = `
    <th>文件</th>
    <th>方法</th>
    <th>参数</th>
    <th>目标转速</th>
    <th>上升时间(ms)</th>
    <th>稳态建立(ms)</th>
    <th>稳态均值</th>
    <th>稳态误差</th>
    <th>稳态标准差</th>
    <th>稳态峰峰值</th>
    <th>最大超调</th>
    <th>总时长(ms)</th>
  `;
}

function renderExperimentMetricNote() {
  if (state.analysisMode === "pll") {
    dom.experimentMetricNote.innerHTML = [
      "<strong>PLL 模式指标说明：</strong>这里重点评价的是估计器原始速度，而不是闭环控制最终效果。",
      "<strong>启动最小速度：</strong>看低速阶段是否出现明显反向毛刺。",
      "<strong>首次稳定为正(ms)：</strong>看原始速度多久才能连续稳定到正确方向。",
      "<strong>切换突跳 / 切换恢复(ms)：</strong>看 start 到 run 切换附近是否平顺。",
      "<strong>稳态原始速度标准差 / 峰峰值：</strong>看估计值本身抖动大小。",
      "<strong>调速段原始速度标准差：</strong>看 500->1000 或 1000->500 变化时原始估计波动是否更大。"
    ].join("<br>");
    return;
  }

  if (state.analysisMode === "switch") {
    dom.experimentMetricNote.innerHTML = [
      "<strong>切换保护模式指标说明：</strong>这里重点评价启动到闭环切换附近是否平顺。",
      "<strong>切换突跳：</strong>越小越说明切换更干净。",
      "<strong>恢复到450rpm时间：</strong>越短说明切换后恢复更快。",
      "<strong>启动最小速度 / 是否有反向毛刺：</strong>用于判断低速误判是否被压住。"
    ].join("<br>");
    return;
  }

  dom.experimentMetricNote.innerHTML = [
    "<strong>滤波模式指标说明：</strong>这里重点评价滤波后速度的系统级表现。",
    "<strong>上升时间 / 稳态建立：</strong>看动态响应速度。",
    "<strong>稳态误差 / 稳态标准差 / 峰峰值：</strong>看稳态跟踪误差和抖动大小。",
    "<strong>最大超调：</strong>看目标变化时是否冲得过头。"
  ].join("<br>");
}

function renderPllExperimentTable() {
  const summaries = state.datasets
    .filter((dataset) => dataset.visible)
    .map((dataset) => buildPllSummary(dataset))
    .filter(Boolean);

  if (!summaries.length) {
    const tr = document.createElement("tr");
    tr.innerHTML = '<td colspan="12">导入包含 raw_speed_rpm、target_speed_rpm、phase 的 CSV 后，这里会自动生成 PLL 估计器指标。</td>';
    dom.experimentTableBody.appendChild(tr);
    return;
  }

  summaries.forEach((summary) => {
    const tr = document.createElement("tr");
    tr.innerHTML = `
      <td>${escapeHtml(summary.datasetLabel)}</td>
      <td>${escapeHtml(summary.methodName)}</td>
      <td>${escapeHtml(summary.paramTag)}</td>
      <td>${formatNumber(summary.targetSpeed)}</td>
      <td>${formatNumber(summary.startupMinSpeed)}</td>
      <td>${formatNumber(summary.firstStablePositiveMs)}</td>
      <td>${formatNumber(summary.switchJump)}</td>
      <td>${formatNumber(summary.switchRecoverMs)}</td>
      <td>${formatNumber(summary.steadyRawStd)}</td>
      <td>${formatNumber(summary.steadyRawPeakToPeak)}</td>
      <td>${formatNumber(summary.rampRawStd)}</td>
      <td>${summary.hasReverseSpike ? "有" : "无"}</td>
    `;
    dom.experimentTableBody.appendChild(tr);
  });
}

function renderSwitchExperimentTable() {
  const summaries = state.datasets
    .filter((dataset) => dataset.visible)
    .map((dataset) => buildSwitchSummary(dataset))
    .filter(Boolean);

  if (!summaries.length) {
    const tr = document.createElement("tr");
    tr.innerHTML = '<td colspan="12">导入包含 phase、final_speed_rpm、raw_speed_rpm 的 CSV 后，这里会自动生成切换保护指标。</td>';
    dom.experimentTableBody.appendChild(tr);
    return;
  }

  summaries.forEach((summary) => {
    const tr = document.createElement("tr");
    tr.innerHTML = `
      <td>${escapeHtml(summary.datasetLabel)}</td>
      <td>${escapeHtml(summary.versionLabel)}</td>
      <td>${escapeHtml(summary.methodName)}</td>
      <td>${escapeHtml(summary.paramTag)}</td>
      <td>${formatNumber(summary.switchTimeMs)}</td>
      <td>${formatNumber(summary.preSwitchSpeed)}</td>
      <td>${formatNumber(summary.postSwitchSpeed)}</td>
      <td>${formatNumber(summary.switchJump)}</td>
      <td>${formatNumber(summary.recover450Ms)}</td>
      <td>${formatNumber(summary.startupMinSpeed)}</td>
      <td>${summary.hasReverseSpike ? "有" : "无"}</td>
      <td>${escapeHtml(summary.note)}</td>
    `;
    dom.experimentTableBody.appendChild(tr);
  });
}

function computeStats(values) {
  if (!values.length) {
    return { mean: NaN, min: NaN, max: NaN, peakToPeak: NaN, std: NaN };
  }
  const mean = values.reduce((sum, value) => sum + value, 0) / values.length;
  const min = Math.min(...values);
  const max = Math.max(...values);
  const variance =
    values.reduce((sum, value) => sum + (value - mean) ** 2, 0) / values.length;
  return {
    mean,
    min,
    max,
    peakToPeak: max - min,
    std: Math.sqrt(variance)
  };
}

function buildExperimentSummary(dataset) {
  const timeColumn = findDatasetColumn(dataset, ["time", "ms"]);
  const filteredColumn = findDatasetColumn(dataset, ["filtered", "speed"], ["final"]);
  const targetColumn = findDatasetColumn(dataset, ["target", "speed"]);

  if (!timeColumn || !filteredColumn || !targetColumn) {
    return null;
  }

  const points = dataset.rows
    .map((row) => {
      const t = row[timeColumn];
      const y = row[filteredColumn];
      const target = row[targetColumn];
      if (!isFiniteNumber(t) || !isFiniteNumber(y) || !isFiniteNumber(target)) {
        return null;
      }
      return { t, y, target };
    })
    .filter(Boolean);

  if (points.length < 8) {
    return null;
  }

  const targetSpeed = pickRepresentativeTarget(points);
  if (!Number.isFinite(targetSpeed) || Math.abs(targetSpeed) < 1e-6) {
    return null;
  }

  const startTime = points[0].t;
  const normalizedPoints = points.map((point) => ({
    ...point,
    t: point.t - startTime
  }));

  const totalDurationMs = normalizedPoints[normalizedPoints.length - 1].t;
  const riseTimeMs = findRiseTime(normalizedPoints, targetSpeed, 0.9);
  const settlingTimeMs = findSettlingTime(normalizedPoints, targetSpeed, 0.05, 8);
  const steadyPoints = pickSteadyStatePoints(normalizedPoints, totalDurationMs);
  const steadyStats = computeStats(steadyPoints.map((point) => point.y));
  const overshoot = computeOvershoot(normalizedPoints, targetSpeed);
  const meta = extractDatasetMeta(dataset);

  return {
    datasetName: dataset.name,
    datasetLabel: buildShortDatasetLabel(dataset),
    methodName: meta.methodName,
    paramTag: meta.paramTag,
    targetSpeed,
    riseTimeMs,
    settlingTimeMs,
    steadyMean: steadyStats.mean,
    steadyError: Number.isFinite(steadyStats.mean) ? steadyStats.mean - targetSpeed : NaN,
    steadyStd: steadyStats.std,
    steadyPeakToPeak: steadyStats.peakToPeak,
    overshoot,
    totalDurationMs
  };
}

function buildPllSummary(dataset) {
  const timeColumn = findDatasetColumn(dataset, ["time", "ms"]);
  const rawColumn = findDatasetColumn(dataset, ["raw", "speed"]);
  const targetColumn = findDatasetColumn(dataset, ["target", "speed"]);
  const phaseColumn = findDatasetColumn(dataset, ["phase"]);

  if (!timeColumn || !rawColumn || !targetColumn) {
    return null;
  }

  const points = dataset.rows
    .map((row) => {
      const t = row[timeColumn];
      const raw = row[rawColumn];
      const target = row[targetColumn];
      const phase = phaseColumn ? String(row[phaseColumn] || "") : "";
      if (!isFiniteNumber(t) || !isFiniteNumber(raw) || !isFiniteNumber(target)) {
        return null;
      }
      return { t, raw, target, phase };
    })
    .filter(Boolean);

  if (points.length < 12) {
    return null;
  }

  const startTime = points[0].t;
  const normalized = points.map((point) => ({
    ...point,
    t: point.t - startTime
  }));

  const targetSpeed = pickRepresentativeTarget(points.map((point) => ({ target: point.target })));
  const startupSegment = normalized.filter((point) => point.phase !== "run");
  const startupMinSpeed = startupSegment.length
    ? Math.min(...startupSegment.map((point) => point.raw))
    : Math.min(...normalized.map((point) => point.raw));

  const firstStablePositiveMs = findFirstStablePositiveTime(normalized, 20, 3);
  const switchMetrics = computeSwitchMetricsFromRaw(normalized);
  const steadyStats = computeRawSteadyStats(normalized, targetSpeed);
  const rampRawStd = computeRampRawStd(normalized, targetSpeed);
  const meta = extractDatasetMeta(dataset);

  return {
    datasetLabel: buildShortDatasetLabel(dataset),
    methodName: meta.methodName,
    paramTag: meta.paramTag,
    targetSpeed,
    startupMinSpeed,
    firstStablePositiveMs,
    switchJump: switchMetrics.switchJump,
    switchRecoverMs: switchMetrics.switchRecoverMs,
    steadyRawStd: steadyStats.std,
    steadyRawPeakToPeak: steadyStats.peakToPeak,
    rampRawStd,
    hasReverseSpike: startupMinSpeed < -1
  };
}

function findFirstStablePositiveTime(points, minSpeed, holdCount) {
  for (let i = 0; i < points.length; i += 1) {
    const window = points.slice(i, i + holdCount);
    if (window.length < holdCount) {
      break;
    }
    const ok = window.every((point) => point.raw >= minSpeed);
    if (ok) {
      return points[i].t;
    }
  }
  return NaN;
}

function computeSwitchMetricsFromRaw(points) {
  let switchIndex = -1;
  for (let i = 1; i < points.length; i += 1) {
    if (points[i - 1].phase !== "run" && points[i].phase === "run") {
      switchIndex = i;
      break;
    }
  }

  if (switchIndex < 1) {
    return { switchJump: NaN, switchRecoverMs: NaN };
  }

  const prev = points[switchIndex - 1];
  const current = points[switchIndex];
  const switchJump = Math.abs(current.raw - prev.raw);
  const recoverPoint = points
    .slice(switchIndex)
    .find((point) => point.raw >= 450);

  return {
    switchJump,
    switchRecoverMs: recoverPoint ? recoverPoint.t - current.t : NaN
  };
}

function computeRawSteadyStats(points, targetSpeed) {
  const targetAbs = Math.abs(targetSpeed);
  const steadyCandidates = points.filter((point) => point.raw >= targetAbs * 0.85);
  const steadyPoints = steadyCandidates.length >= 8
    ? steadyCandidates.slice(Math.floor(steadyCandidates.length * 0.6))
    : points.slice(Math.max(points.length - 20, 0));
  return computeStats(steadyPoints.map((point) => point.raw));
}

function computeRampRawStd(points, targetSpeed) {
  const targetAbs = Math.abs(targetSpeed);
  const rampPoints = points.filter(
    (point) => point.raw >= targetAbs * 0.3 && point.raw <= targetAbs * 0.8
  );
  return computeStats(rampPoints.map((point) => point.raw)).std;
}

function buildSwitchSummary(dataset) {
  const timeColumn = findDatasetColumn(dataset, ["time", "ms"]);
  const rawColumn = findDatasetColumn(dataset, ["raw", "speed"]);
  const finalColumn = findDatasetColumn(dataset, ["final", "speed"]) || findDatasetColumn(dataset, ["filtered", "speed"]);
  const phaseColumn = findDatasetColumn(dataset, ["phase"]);

  if (!timeColumn || !finalColumn || !phaseColumn) {
    return null;
  }

  const points = dataset.rows
    .map((row, index) => {
      const t = row[timeColumn];
      const finalSpeed = row[finalColumn];
      const rawSpeed = rawColumn ? row[rawColumn] : finalSpeed;
      const phase = row[phaseColumn];
      if (!isFiniteNumber(t) || !isFiniteNumber(finalSpeed)) {
        return null;
      }
      return {
        index,
        t,
        raw: isFiniteNumber(rawSpeed) ? rawSpeed : finalSpeed,
        final: finalSpeed,
        phase: String(phase || "")
      };
    })
    .filter(Boolean);

  if (points.length < 8) {
    return null;
  }

  const startTime = points[0].t;
  const normalized = points.map((point) => ({
    ...point,
    t: point.t - startTime
  }));

  let switchIndex = -1;
  for (let i = 1; i < normalized.length; i += 1) {
    if (normalized[i - 1].phase !== "run" && normalized[i].phase === "run") {
      switchIndex = i;
      break;
    }
  }

  if (switchIndex < 1) {
    return null;
  }

  const switchPoint = normalized[switchIndex];
  const prevPoint = normalized[switchIndex - 1];
  const recoverPoint = normalized.slice(switchIndex).find((point) => Math.abs(point.final) >= 450);
  const startupSegment = normalized.filter((point) => point.phase === "start");
  const startupMinSpeed = startupSegment.length
    ? Math.min(...startupSegment.map((point) => Math.min(point.raw, point.final)))
    : Math.min(...normalized.map((point) => Math.min(point.raw, point.final)));
  const meta = extractDatasetMeta(dataset);

  return {
    datasetLabel: buildShortDatasetLabel(dataset),
    versionLabel: getSwitchVersionLabel(meta),
    methodName: meta.methodName,
    paramTag: meta.paramTag,
    switchTimeMs: switchPoint.t,
    preSwitchSpeed: prevPoint.final,
    postSwitchSpeed: switchPoint.final,
    switchJump: Math.abs(switchPoint.final - prevPoint.final),
    recover450Ms: recoverPoint ? recoverPoint.t - switchPoint.t : NaN,
    startupMinSpeed,
    hasReverseSpike: startupMinSpeed < -1,
    note: getSwitchVersionNote(meta)
  };
}

function getSwitchVersionLabel(meta) {
  if (meta.rawParamTag === "protect_off" || meta.methodCode === "PLL_FIX") {
    return "原版基线";
  }
  if (meta.rawParamTag === "protect_on" || meta.methodCode === "PLL_SPLIT") {
    return "改进版";
  }
  return "未分类";
}

function getSwitchVersionNote(meta) {
  if (meta.rawParamTag === "protect_off") {
    return "关闭启动切换保护，其余参数固定";
  }
  if (meta.rawParamTag === "protect_on") {
    return "开启启动切换保护，其余参数固定";
  }
  if (meta.methodCode === "PLL_FIX") {
    return "固定参数 PLL，对照基线";
  }
  if (meta.methodCode === "PLL_SPLIT") {
    return "分程参数 + 启动保护相关改进";
  }
  return "请结合文件名确认";
}

function findDatasetColumn(dataset, includeKeywords, excludeKeywords = []) {
  const include = includeKeywords.map((item) => item.toLowerCase());
  const exclude = excludeKeywords.map((item) => item.toLowerCase());

  return (
    dataset.headers.find((header) => {
      const lower = header.toLowerCase();
      return include.every((keyword) => lower.includes(keyword)) && !exclude.some((keyword) => lower.includes(keyword));
    }) || ""
  );
}

function pickRepresentativeTarget(points) {
  const stats = computeStats(points.map((point) => point.target));
  return stats.mean;
}

function findRiseTime(points, targetSpeed, ratio) {
  const threshold = targetSpeed * ratio;
  const point = points.find((item) => item.y >= threshold);
  return point ? point.t : NaN;
}

function findSettlingTime(points, targetSpeed, toleranceRatio, holdCount) {
  const tolerance = Math.abs(targetSpeed) * toleranceRatio;
  for (let i = 0; i < points.length; i += 1) {
    let ok = true;
    for (let j = i; j < Math.min(points.length, i + holdCount); j += 1) {
      if (Math.abs(points[j].y - targetSpeed) > tolerance) {
        ok = false;
        break;
      }
    }
    if (ok && i + holdCount <= points.length) {
      return points[i].t;
    }
  }
  return NaN;
}

function pickSteadyStatePoints(points, totalDurationMs) {
  const startTime = totalDurationMs * 0.65;
  const result = points.filter((point) => point.t >= startTime);
  return result.length >= 5 ? result : points.slice(Math.max(points.length - 10, 0));
}

function computeOvershoot(points, targetSpeed) {
  const maxValue = Math.max(...points.map((point) => point.y));
  return maxValue - targetSpeed;
}

function extractDatasetMeta(dataset) {
  const firstRow = dataset.rows[0] || {};
  const methodCode = String(firstRow.method_name || firstRow.methodName || "未知方法");
  const rawParamTag = String(firstRow.param_tag || firstRow.paramName || firstRow.param_tag || "无参数");
  return {
    methodCode,
    methodName: methodLabelMap[methodCode] || methodCode,
    rawParamTag,
    paramTag: translateParamTag(methodCode, rawParamTag),
    shortParamTag: buildShortParamTag(methodCode, rawParamTag)
  };
}

function renderChart() {
  if (state.layoutMode === "small-multiples") {
    dom.canvas.classList.add("hidden");
    dom.smallMultiplesGrid.classList.remove("hidden");
    renderSmallMultiples();
    return;
  }

  dom.canvas.classList.remove("hidden");
  dom.smallMultiplesGrid.classList.add("hidden");
  dom.smallMultiplesGrid.innerHTML = "";

  const seriesList = collectSeries();
  const canvas = dom.canvas;
  const width = canvas.width;
  const height = canvas.height;
  ctx.clearRect(0, 0, width, height);

  drawCanvasBackground(width, height);

  if (!seriesList.length || !state.xColumn || !state.yColumn) {
    drawEmptyState(width, height);
    return;
  }

  const filteredSeries = seriesList
    .map((series) => ({
      ...series,
      rows: filterPointsByWindow(series.rows)
    }))
    .filter((series) => series.rows.length > 0);

  const targetSeries = getOverlayTargetSeries(filteredSeries);
  const plotSeries = filteredSeries
    .filter((series) => series.axis !== "y2")
    .concat(targetSeries);

  if (!plotSeries.length) {
    drawNoWindowData(width, height);
    return;
  }

  const padding = { top: 66, right: state.showY2 && state.y2Column ? 100 : 40, bottom: 58, left: 78 };
  const plotWidth = width - padding.left - padding.right;
  const plotHeight = height - padding.top - padding.bottom;

  const xValues = plotSeries.flatMap((series) => series.rows.map((point) => point.x));
  const y1Values = plotSeries.flatMap((series) => series.rows.map((point) => point.y));

  const xDomain = pickDomain(xValues, state.ranges.xMin, state.ranges.xMax);
  const yDomain = pickDomain(y1Values, state.ranges.yMin, state.ranges.yMax);
  const y2Domain = null;

  drawGridAndAxes(padding, plotWidth, plotHeight, xDomain, yDomain, y2Domain);

  plotSeries.forEach((series) => {
    drawSeries(series, padding, plotWidth, plotHeight, xDomain, yDomain);
  });

  drawAxisTitles(padding, width, height);
  if (state.showLegend) {
    drawLegend(plotSeries, width);
  }
}

function drawCanvasBackground(width, height) {
  const gradient = ctx.createLinearGradient(0, 0, 0, height);
  gradient.addColorStop(0, "rgba(255,255,255,0.95)");
  gradient.addColorStop(1, "rgba(247,239,226,0.98)");
  ctx.fillStyle = gradient;
  ctx.fillRect(0, 0, width, height);
}

function drawEmptyState(width, height) {
  ctx.fillStyle = "#6f6258";
  ctx.textAlign = "center";
  ctx.font = "600 30px Segoe UI";
  ctx.fillText("导入 CSV 后开始对比", width / 2, height / 2 - 10);
  ctx.font = "18px Segoe UI";
  ctx.fillText("建议 CSV 至少包含时间列、原始速度列、滤波速度列和目标速度列", width / 2, height / 2 + 26);
}

function drawNoWindowData(width, height) {
  ctx.fillStyle = "#6f6258";
  ctx.textAlign = "center";
  ctx.font = "600 28px Segoe UI";
  ctx.fillText("当前窗口内没有数据", width / 2, height / 2);
}

function drawGridAndAxes(padding, plotWidth, plotHeight, xDomain, yDomain, y2Domain) {
  const steps = 6;
  ctx.save();
  ctx.strokeStyle = "rgba(31,23,17,0.1)";
  ctx.lineWidth = 1;
  ctx.fillStyle = "#6f6258";
  ctx.font = "14px Segoe UI";

  for (let i = 0; i <= steps; i += 1) {
    const x = padding.left + (plotWidth * i) / steps;
    ctx.beginPath();
    ctx.moveTo(x, padding.top);
    ctx.lineTo(x, padding.top + plotHeight);
    ctx.stroke();

    const value = lerp(xDomain.min, xDomain.max, i / steps);
    ctx.textAlign = "center";
    ctx.fillText(formatNumber(value), x, padding.top + plotHeight + 24);
  }

  for (let i = 0; i <= steps; i += 1) {
    const y = padding.top + (plotHeight * i) / steps;
    ctx.beginPath();
    ctx.moveTo(padding.left, y);
    ctx.lineTo(padding.left + plotWidth, y);
    ctx.stroke();

    const value = lerp(yDomain.max, yDomain.min, i / steps);
    ctx.textAlign = "right";
    ctx.fillText(formatNumber(value), padding.left - 12, y + 5);

    if (y2Domain) {
      const y2Value = lerp(y2Domain.max, y2Domain.min, i / steps);
      ctx.textAlign = "left";
      ctx.fillText(formatNumber(y2Value), padding.left + plotWidth + 12, y + 5);
    }
  }

  ctx.strokeStyle = "rgba(31,23,17,0.28)";
  ctx.lineWidth = 1.4;
  ctx.strokeRect(padding.left, padding.top, plotWidth, plotHeight);
  ctx.restore();
}

function drawSeries(series, padding, plotWidth, plotHeight, xDomain, yDomain) {
  if (!series.rows.length) {
    return;
  }

  ctx.save();
  ctx.strokeStyle = series.axis === "y2" ? withAlpha(series.color, 0.58) : series.color;
  ctx.lineWidth = series.axis === "y2" ? 2 : 2.8;
  ctx.setLineDash(series.lineDash);
  ctx.beginPath();

  let penDown = false;
  series.rows.forEach((point) => {
    const x = mapValue(point.x, xDomain.min, xDomain.max, padding.left, padding.left + plotWidth);
    const y = mapValue(point.y, yDomain.min, yDomain.max, padding.top + plotHeight, padding.top);
    if (!penDown) {
      ctx.moveTo(x, y);
      penDown = true;
    } else {
      ctx.lineTo(x, y);
    }

    if (point.breakAfter) {
      penDown = false;
    }
  });

  ctx.stroke();

  if (state.showPoints) {
    ctx.fillStyle = series.color;
    ctx.setLineDash([]);
    series.rows.forEach((point) => {
      const x = mapValue(point.x, xDomain.min, xDomain.max, padding.left, padding.left + plotWidth);
      const y = mapValue(point.y, yDomain.min, yDomain.max, padding.top + plotHeight, padding.top);
      ctx.beginPath();
      ctx.arc(x, y, 2.7, 0, Math.PI * 2);
      ctx.fill();
    });
  }

  ctx.restore();
}

function drawAxisTitles(padding, width, height) {
  ctx.save();
  ctx.fillStyle = "#1f1711";
  ctx.font = "600 18px Segoe UI";
  ctx.textAlign = "center";
  ctx.fillText(`${state.xColumn || "X"} 轴`, width / 2, height - 14);

  ctx.translate(24, height / 2);
  ctx.rotate(-Math.PI / 2);
  ctx.fillText([state.yColumn, state.compareColumn].filter(Boolean).join(" + ") || "Y 轴", 0, 0);
  ctx.restore();

}

function drawLegend(seriesList, width) {
  const orderedSeries = orderLegendSeries(seriesList);
  ctx.save();
  ctx.font = "15px Segoe UI";
  ctx.textAlign = "left";
  let x = 40;
  let y = 28;

  orderedSeries.forEach((series) => {
    ctx.fillStyle = series.color;
    ctx.fillRect(x, y - 10, 18, 10);
    ctx.fillStyle = "#1f1711";
    const label = getLegendLabel(series);
    ctx.fillText(label, x + 26, y);
    x += Math.max(180, ctx.measureText(label).width + 56);
    if (x > width - 280) {
      x = 40;
      y += 24;
    }
  });
  ctx.restore();
}

function orderLegendSeries(seriesList) {
  return [...seriesList].sort((a, b) => {
    if (a.axis === "y2" && b.axis !== "y2") {
      return -1;
    }
    if (a.axis !== "y2" && b.axis === "y2") {
      return 1;
    }
    return a.datasetIndex - b.datasetIndex;
  });
}

function getOverlayTargetSeries(seriesList) {
  if (!state.showY2 || !state.y2Column) {
    return [];
  }

  const firstTarget = seriesList.find((series) => series.axis === "y2");
  return firstTarget ? [firstTarget] : [];
}

function getLegendLabel(series) {
  if (series.axis === "y2") {
    return state.analysisMode === "pll" ? "目标转速（虚线）" : "目标速度（虚线）";
  }

  const column = String(series.column || "").toLowerCase();
  if (state.analysisMode === "pll") {
    if (column.includes("raw") && column.includes("speed")) {
      return `${series.datasetLabel} · 原始速度（实线）`;
    }
    if (column.includes("final") && column.includes("speed")) {
      return `${series.datasetLabel} · 控制使用速度（辅助虚线）`;
    }
    if (column.includes("filtered") && column.includes("speed")) {
      return `${series.datasetLabel} · 处理后速度（辅助虚线）`;
    }
  }

  if (column.includes("raw") && column.includes("speed")) {
    return `${series.datasetLabel} · 原始速度（虚线）`;
  }

  if (column.includes("filtered") && column.includes("speed")) {
    return `${series.datasetLabel} · 滤波速度（实线）`;
  }

  if (column.includes("final") && column.includes("speed")) {
    return `${series.datasetLabel} · 控制使用速度（实线）`;
  }

  return `${series.datasetLabel} · ${translateColumnLabel(series.column)}`;
}

function filterPointsByWindow(rows) {
  return rows.filter((point) => {
    const withinXMin = state.ranges.xMin == null || point.x >= state.ranges.xMin;
    const withinXMax = state.ranges.xMax == null || point.x <= state.ranges.xMax;
    const withinYMin = state.ranges.yMin == null || point.y >= state.ranges.yMin;
    const withinYMax = state.ranges.yMax == null || point.y <= state.ranges.yMax;
    return withinXMin && withinXMax && withinYMin && withinYMax;
  });
}

function pickDomain(values, manualMin, manualMax) {
  const min = manualMin != null ? manualMin : Math.min(...values);
  const max = manualMax != null ? manualMax : Math.max(...values);

  if (min === max) {
    return { min: min - 1, max: max + 1 };
  }

  const padding = (max - min) * 0.05;
  return {
    min: manualMin != null ? manualMin : min - padding,
    max: manualMax != null ? manualMax : max + padding
  };
}

function mapValue(value, domainMin, domainMax, rangeMin, rangeMax) {
  return rangeMin + ((value - domainMin) / (domainMax - domainMin)) * (rangeMax - rangeMin);
}

function lerp(min, max, ratio) {
  return min + (max - min) * ratio;
}

function withAlpha(hexColor, alpha) {
  const hex = hexColor.replace("#", "");
  const bigint = parseInt(hex, 16);
  const r = (bigint >> 16) & 255;
  const g = (bigint >> 8) & 255;
  const b = bigint & 255;
  return `rgba(${r}, ${g}, ${b}, ${alpha})`;
}

function formatNumber(value) {
  if (!Number.isFinite(value)) {
    return "-";
  }
  if (Math.abs(value) >= 1000 || (Math.abs(value) > 0 && Math.abs(value) < 0.01)) {
    return value.toExponential(2);
  }
  return value.toFixed(3).replace(/\.?0+$/, "");
}

function isFiniteNumber(value) {
  return typeof value === "number" && Number.isFinite(value);
}

function escapeHtml(value) {
  return String(value)
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;");
}

function clearAll() {
  state.datasets = [];
  state.columns = [];
  state.analysisMode = "filter";
  state.xColumn = "";
  state.yColumn = "";
  state.y2Column = "";
  state.compareColumn = "";
  state.layoutMode = "overlay";
  resetRanges();
  renderAll();
}

function exportPng() {
  const link = document.createElement("a");
  link.download = `mc-speed-compare-${Date.now()}.png`;
  link.href = dom.canvas.toDataURL("image/png");
  link.click();
}

function applyPresetRawFiltered() {
  if (state.analysisMode === "pll") {
    applyPresetColumns({
      xColumn: findBestColumn(["time", "ms"], []),
      yColumn: findBestColumn(["raw", "speed"], []),
      compareColumn: "",
      y2Column: findBestColumn(["target", "speed"], [])
    });
    clearPresetRanges();
    renderAll();
    return;
  }

  if (state.analysisMode === "switch") {
    applyPresetColumns({
      xColumn: findBestColumn(["time", "ms"], []),
      yColumn: findBestColumn(["raw", "speed"], []),
      compareColumn: findBestColumn(["final", "speed"], []) || findBestColumn(["filtered", "speed"], []),
      y2Column: findBestColumn(["target", "speed"], [])
    });
    clearPresetRanges();
    renderAll();
    return;
  }

  applyPresetColumns({
    xColumn: findBestColumn(["time", "ms"], []),
    yColumn: findBestColumn(["raw", "speed"], []),
    compareColumn: findBestColumn(["filtered", "speed"], []),
    y2Column: findBestColumn(["target", "speed"], [])
  });
  clearPresetRanges();
  renderAll();
}

function applyPresetFilteredTarget() {
  if (state.analysisMode === "pll") {
    applyPresetColumns({
      xColumn: findBestColumn(["time", "ms"], []),
      yColumn: findBestColumn(["pll", "kp"], []) || findBestColumn(["final", "speed"], []),
      compareColumn: findBestColumn(["pll", "ki"], []),
      y2Column: findBestColumn(["pll", "stage"], [])
    });
    clearPresetRanges();
    renderAll();
    return;
  }

  if (state.analysisMode === "switch") {
    applyPresetColumns({
      xColumn: findBestColumn(["time", "ms"], []),
      yColumn: findBestColumn(["final", "speed"], []) || findBestColumn(["filtered", "speed"], []),
      compareColumn: "",
      y2Column: findBestColumn(["target", "speed"], [])
    });
    clearPresetRanges();
    renderAll();
    return;
  }

  applyPresetColumns({
    xColumn: findBestColumn(["time", "ms"], []),
    yColumn: findBestColumn(["filtered", "speed"], []),
    compareColumn: "",
    y2Column: findBestColumn(["target", "speed"], [])
  });
  clearPresetRanges();
  renderAll();
}

function applyPresetStartup() {
  if (state.analysisMode === "pll") {
    applyPresetColumns({
      xColumn: findBestColumn(["time", "ms"], []),
      yColumn: findBestColumn(["raw", "speed"], []),
      compareColumn: "",
      y2Column: findBestColumn(["target", "speed"], [])
    });
  } else if (state.analysisMode === "switch") {
    applyPresetColumns({
      xColumn: findBestColumn(["time", "ms"], []),
      yColumn: findBestColumn(["raw", "speed"], []),
      compareColumn: findBestColumn(["final", "speed"], []) || findBestColumn(["filtered", "speed"], []),
      y2Column: findBestColumn(["target", "speed"], [])
    });
  } else {
  applyPresetColumns({
    xColumn: findBestColumn(["time", "ms"], []),
    yColumn: findBestColumn(["filtered", "speed"], []),
    compareColumn: "",
    y2Column: findBestColumn(["target", "speed"], [])
  });
  }
  if (!state.xColumn) {
    return;
  }
  const startupWindow = inferStartupWindow();
  if (startupWindow) {
    state.ranges.xMin = startupWindow.xMin;
    state.ranges.xMax = startupWindow.xMax;
  } else {
    state.ranges.xMin = 0;
    state.ranges.xMax = 500;
  }
  state.ranges.yMin = null;
  state.ranges.yMax = null;
  syncRangeInputs();
  renderAll();
}

function applyPresetSteadyState() {
  if (state.analysisMode === "pll") {
    applyPresetColumns({
      xColumn: findBestColumn(["time", "ms"], []),
      yColumn: findBestColumn(["raw", "speed"], []),
      compareColumn: "",
      y2Column: findBestColumn(["target", "speed"], [])
    });
  } else if (state.analysisMode === "switch") {
    applyPresetColumns({
      xColumn: findBestColumn(["time", "ms"], []),
      yColumn: findBestColumn(["final", "speed"], []) || findBestColumn(["filtered", "speed"], []),
      compareColumn: "",
      y2Column: findBestColumn(["target", "speed"], [])
    });
  } else {
  applyPresetColumns({
    xColumn: findBestColumn(["time", "ms"], []),
    yColumn: findBestColumn(["filtered", "speed"], []),
    compareColumn: "",
    y2Column: findBestColumn(["target", "speed"], [])
  });
  }
  const xDomain = inferVisibleXDomain();
  if (!xDomain) {
    return;
  }
  const span = xDomain.max - xDomain.min;
  const start = xDomain.min + span * 0.7;
  state.ranges.xMin = Number.isFinite(start) ? start : null;
  state.ranges.xMax = xDomain.max;
  state.ranges.yMin = null;
  state.ranges.yMax = null;
  syncRangeInputs();
  renderAll();
}

function applyModeDefaults() {
  if (state.analysisMode === "pll") {
    applyPresetColumns({
      xColumn: findBestColumn(["time", "ms"], []),
      yColumn: findBestColumn(["raw", "speed"], []),
      compareColumn: "",
      y2Column: findBestColumn(["target", "speed"], [])
    });
    clearPresetRanges();
    return;
  }

  if (state.analysisMode === "switch") {
    applyPresetColumns({
      xColumn: findBestColumn(["time", "ms"], []),
      yColumn: findBestColumn(["raw", "speed"], []),
      compareColumn: findBestColumn(["final", "speed"], []) || findBestColumn(["filtered", "speed"], []),
      y2Column: findBestColumn(["target", "speed"], [])
    });
    clearPresetRanges();
    return;
  }

  applyPresetColumns({
    xColumn: findBestColumn(["time", "ms"], []),
    yColumn: findBestColumn(["filtered", "speed"], []),
    compareColumn: "",
    y2Column: findBestColumn(["target", "speed"], [])
  });
  clearPresetRanges();
}

function applyPresetColumns({ xColumn, yColumn, compareColumn, y2Column }) {
  state.xColumn = xColumn || state.xColumn || state.columns[0] || "";
  state.yColumn = yColumn || state.yColumn || "";
  state.compareColumn =
    compareColumn && compareColumn !== state.yColumn ? compareColumn : "";
  state.y2Column =
    y2Column && y2Column !== state.yColumn && y2Column !== state.compareColumn
      ? y2Column
      : "";
  state.showY2 = Boolean(state.y2Column);
}

function clearPresetRanges() {
  state.ranges.xMin = null;
  state.ranges.xMax = null;
  state.ranges.yMin = null;
  state.ranges.yMax = null;
  syncRangeInputs();
}

function inferVisibleXDomain() {
  const series = collectSeries();
  const xValues = series.flatMap((item) => item.rows.map((point) => point.x));
  if (!xValues.length) {
    return null;
  }
  return {
    min: Math.min(...xValues),
    max: Math.max(...xValues)
  };
}

function inferStartupWindow() {
  const visibleDatasets = state.datasets.filter((dataset) => dataset.visible);
  const eventXs = visibleDatasets
    .map((dataset) => detectStartupEventX(dataset))
    .filter((value) => Number.isFinite(value));

  if (!eventXs.length) {
    return null;
  }

  const eventX = Math.min(...eventXs);
  const xDomain = inferVisibleXDomain();
  if (!xDomain) {
    return null;
  }

  const span = xDomain.max - xDomain.min;
  const preRoll = Math.min(200, Math.max(80, span * 0.03));
  const postRoll = Math.min(900, Math.max(450, span * 0.12));

  return {
    xMin: Math.max(xDomain.min, eventX - preRoll),
    xMax: Math.min(xDomain.max, eventX + postRoll)
  };
}

function detectStartupEventX(dataset) {
  const filteredColumn = findDatasetColumn(dataset, ["filtered", "speed"], ["final"]);
  const rawColumn = findDatasetColumn(dataset, ["raw", "speed"]);
  const rows = buildDatasetXyRows(dataset, [filteredColumn, rawColumn]);

  if (rows.length < 4) {
    return NaN;
  }

  const baselineCount = Math.min(8, rows.length);
  const baselineValues = rows
    .slice(0, baselineCount)
    .flatMap((row) => row.values)
    .filter(isFiniteNumber);
  const baseline = baselineValues.length
    ? baselineValues.reduce((sum, value) => sum + value, 0) / baselineValues.length
    : 0;

  for (let i = 0; i < rows.length; i += 1) {
    const window = rows.slice(i, i + 3);
    if (window.length < 3) {
      break;
    }

    const activeCount = window.reduce((count, row) => {
      const hasActivity = row.values.some((value) => Math.abs(value - baseline) >= 8);
      return count + (hasActivity ? 1 : 0);
    }, 0);

    if (activeCount >= 2) {
      return rows[i].x;
    }
  }

  const fallback = rows.find((row) =>
    row.values.some((value) => Math.abs(value - baseline) >= 8)
  );
  return fallback ? fallback.x : NaN;
}

function buildDatasetXyRows(dataset, yColumns) {
  if (!dataset.headers.includes(state.xColumn)) {
    return [];
  }

  const validYColumns = yColumns.filter(
    (column) => column && dataset.headers.includes(column)
  );
  if (!validYColumns.length) {
    return [];
  }

  const rows = dataset.rows
    .map((row) => {
      const x = row[state.xColumn];
      if (!isFiniteNumber(x)) {
        return null;
      }

      const values = validYColumns
        .map((column) => row[column])
        .filter(isFiniteNumber);

      if (!values.length) {
        return null;
      }

      return { x, values };
    })
    .filter(Boolean);

  if (state.normalizeTime && rows.length) {
    const offset = rows[0].x;
    rows.forEach((point) => {
      point.x -= offset;
    });
  }

  return rows;
}

function findBestColumn(keywords, exclude) {
  const lowerKeywords = keywords.map((item) => item.toLowerCase());
  const match = state.columns.find((column) => {
    if (exclude.includes(column)) {
      return false;
    }
    const lower = column.toLowerCase();
    return lowerKeywords.every((keyword) => lower.includes(keyword));
  });
  return match || "";
}

function syncRangeInputs() {
  dom.xMinInput.value = state.ranges.xMin ?? "";
  dom.xMaxInput.value = state.ranges.xMax ?? "";
  dom.yMinInput.value = state.ranges.yMin ?? "";
  dom.yMaxInput.value = state.ranges.yMax ?? "";
}

function buildShortDatasetLabel(dataset) {
  const meta = extractDatasetMeta(dataset);
  const methodLabel = shortMethodLabelMap[meta.methodCode] || meta.methodCode || meta.methodName;
  const paramText =
    meta.shortParamTag && meta.shortParamTag !== "默认"
      ? `（${meta.shortParamTag}）`
      : "";
  return `${methodLabel}${paramText}`;
}

function buildFullDatasetLabel(dataset) {
  const meta = extractDatasetMeta(dataset);
  const paramText =
    meta.paramTag && meta.paramTag !== "默认参数"
      ? `（${meta.paramTag}）`
      : "";
  return `${meta.methodName}${paramText}`;
}

function buildShortParamTag(methodCode, rawParamTag) {
  const tag = String(rawParamTag || "").trim();
  if (!tag || tag === "无参数" || tag === "none") {
    return "默认";
  }

  if (methodCode === "LPF1") {
    const match = tag.match(/^shift(\d+)$/i);
    if (match) {
      return `s=${match[1]}`;
    }
  }

  if (methodCode === "MOVAVG8") {
    const match = tag.match(/^n(\d+)$/i);
    if (match) {
      return `n=${match[1]}`;
    }
  }

  if (methodCode === "WMA4") {
    const match = tag.match(/^w([\d-]+)$/i);
    if (match) {
      return `w=${match[1]}`;
    }
  }

  if (methodCode === "ADALPF") {
    const match = tag.match(/^(\d+)to(\d+)_h(\d+)_(\d+)_n(\d+)$/i);
    if (match) {
      const [, fastShift, slowShift, enterRpm, exitRpm, confirmN] = match;
      return `${fastShift}/${slowShift}, ${enterRpm}/${exitRpm}, n${confirmN}`;
    }
  }

  if (methodCode === "PLL_FIX") {
    const match = tag.match(/^kp(\d+)_ki(\d+)$/i);
    if (match) {
      return `Kp${match[1]} Ki${match[2]}`;
    }
  }

  if (methodCode === "PLL_SPLIT") {
    const match = tag.match(
      /^fast(\d+)_(\d+)_slow(\d+)_(\d+)_e(\d+)x(\d+)_n(\d+)$/i
    );
    if (match) {
      const [, fastKp, fastKi, slowKp, slowKi, enterRpm, exitRpm, confirmN] = match;
      return `${fastKp}/${fastKi}->${slowKp}/${slowKi}, ${enterRpm}/${exitRpm}, n${confirmN}`;
    }
  }

  if (tag === "protect_off") {
    return "保护关";
  }

  if (tag === "protect_on") {
    return "保护开";
  }

  return tag.replaceAll("_", " ");
}

function buildDatasetLabel(dataset) {
  return buildShortDatasetLabel(dataset);
}

function renderLegendExplainBox() {
  const visibleDatasets = state.datasets.filter((dataset) => dataset.visible);
  if (!visibleDatasets.length) {
    dom.legendExplainList.className = "legend-explain-list empty-state";
    dom.legendExplainList.textContent = "导入 CSV 后，这里会显示每条方法曲线的中文说明。";
    return;
  }

  dom.legendExplainList.className = "legend-explain-list";
  dom.legendExplainList.innerHTML = "";

  visibleDatasets.forEach((dataset) => {
    const meta = extractDatasetMeta(dataset);
    const card = document.createElement("div");
    card.className = "legend-explain-card";

    const top = document.createElement("div");
    top.className = "legend-explain-top";

    const swatch = document.createElement("span");
    swatch.className = "swatch";
    swatch.style.background = dataset.color;

    const shortTitle = document.createElement("div");
    shortTitle.className = "legend-explain-short";
    shortTitle.textContent = buildShortDatasetLabel(dataset);

    top.append(swatch, shortTitle);

    const method = document.createElement("div");
    method.className = "legend-explain-method";
    method.textContent = `${meta.methodName} | 图中短标题：${buildShortDatasetLabel(dataset)}`;

    const detail = document.createElement("div");
    detail.className = "legend-explain-detail";
    const detailLines = [`<strong>参数说明：</strong>${escapeHtml(meta.paramTag)}`];
    if (state.analysisMode === "pll") {
      detailLines.push("<strong>线型说明：</strong>目标转速为虚线，表示控制命令值；原始速度为实线，表示观测器/估计器直接输出。PLL 模式默认不显示滤波后速度和控制使用速度，避免把后级保护/滤波影响带进 PI 参数优劣比较。");
    } else if (state.analysisMode === "filter") {
      detailLines.push("<strong>线型说明：</strong>原始速度为虚线，滤波速度为实线，目标速度为虚线。");
    }
    if (dataset.note) {
      detailLines.push(`<strong>备注：</strong>${escapeHtml(dataset.note)}`);
    }
    detail.innerHTML = detailLines.join("<br>");

    const rawFile = document.createElement("div");
    rawFile.className = "legend-explain-file";
    rawFile.innerHTML = [
      `<strong>原始文件：</strong>${escapeHtml(dataset.name)}`,
      `<strong>原始参数码：</strong>${escapeHtml(meta.rawParamTag)}`
    ].join("<br>");

    card.append(top, method, detail, rawFile);
    dom.legendExplainList.appendChild(card);
  });
}

function translateParamTag(methodCode, rawParamTag) {
  const tag = String(rawParamTag || "").trim();
  if (!tag || tag === "无参数" || tag === "none") {
    return "默认参数";
  }

  if (methodCode === "LPF1") {
    const match = tag.match(/^shift(\d+)$/i);
    if (match) {
      return `低通强度 shift=${match[1]}`;
    }
  }

  if (methodCode === "MOVAVG8") {
    const match = tag.match(/^n(\d+)$/i);
    if (match) {
      return `滑动窗口 ${match[1]} 点`;
    }
  }

  if (methodCode === "WMA4") {
    const match = tag.match(/^w([\d-]+)$/i);
    if (match) {
      return `权重 ${match[1].split("-").join(":")}`;
    }
  }

  if (methodCode === "ADALPF") {
    const match = tag.match(/^(\d+)to(\d+)_h(\d+)_(\d+)_n(\d+)$/i);
    if (match) {
      const [, fastShift, slowShift, enterRpm, exitRpm, confirmN] = match;
      return `快档shift=${fastShift}，慢档shift=${slowShift}，进入${enterRpm}rpm，退出${exitRpm}rpm，连续${confirmN}拍`;
    }
  }

  if (methodCode === "PLL_FIX") {
    const match = tag.match(/^kp(\d+)_ki(\d+)$/i);
    if (match) {
      return `固定参数 Kp=${match[1]}，Ki=${match[2]}`;
    }
  }

  if (methodCode === "PLL_SPLIT") {
    const match = tag.match(
      /^fast(\d+)_(\d+)_slow(\d+)_(\d+)_e(\d+)x(\d+)_n(\d+)$/i
    );
    if (match) {
      const [, fastKp, fastKi, slowKp, slowKi, enterRpm, exitRpm, confirmN] = match;
      return `快段Kp=${fastKp} Ki=${fastKi}，慢段Kp=${slowKp} Ki=${slowKi}，门限${enterRpm}/${exitRpm}rpm，连续${confirmN}拍`;
    }
  }

  if (tag === "protect_off") {
    return "关闭启动切换保护，其他参数保持一致";
  }

  if (tag === "protect_on") {
    return "开启启动切换保护，其他参数保持一致";
  }

  return tag.replaceAll("_", " ");
}

function translateColumnLabel(column) {
  const map = {
    time_ms: "时间",
    target_speed_rpm: "目标转速",
    raw_speed_rpm: "原始速度",
    filtered_speed_rpm: "滤波速度",
    final_speed_rpm: "控制使用速度",
    pll_kp: "PLL 比例系数 Kp",
    pll_ki: "PLL 积分系数 Ki",
    pll_stage: "PLL 分程阶段",
    pll_split_enable: "PLL 分程使能",
    sample_index: "采样序号"
  };
  return map[column] || column;
}

function renderSmallMultiples() {
  const baseSeries = collectSeries()
    .map((series) => ({
      ...series,
      rows: filterPointsByWindow(series.rows)
    }))
    .filter((series) => series.rows.length > 0);

  dom.smallMultiplesGrid.innerHTML = "";

  if (!baseSeries.length) {
    const empty = document.createElement("div");
    empty.className = "empty-state";
    empty.textContent = "当前没有可用于多图并列显示的数据";
    dom.smallMultiplesGrid.appendChild(empty);
    return;
  }

  const groupedSeries = groupSeriesByDataset(baseSeries);

  groupedSeries.forEach((datasetSeries) => {
    const primarySeries = datasetSeries.find((series) => series.column === state.yColumn) || datasetSeries[0];
    const targetSeries = datasetSeries.find((series) => series.axis === "y2");
    const dataset = state.datasets.find((item) => item.id === primarySeries?.datasetId);
    const summary = dataset ? buildExperimentSummary(dataset) : null;

    if (!primarySeries) {
      return;
    }

    const card = document.createElement("section");
    card.className = "mini-chart-card";

    const title = document.createElement("div");
    title.className = "mini-chart-title";
    title.textContent = primarySeries.datasetLabel;

    const sub = document.createElement("div");
    sub.className = "mini-chart-subtitle";
    sub.textContent = targetSeries
      ? `${translateColumnLabel(primarySeries.column)} + ${translateColumnLabel(targetSeries.column)}`
      : translateColumnLabel(primarySeries.column);

    const badge = document.createElement("div");
    badge.className = "mini-chart-badge";
    badge.textContent = summary
      ? `rise ${formatNumber(summary.riseTimeMs)} ms | std ${formatNumber(summary.steadyStd)} rpm`
      : "指标计算中";

    const legend = document.createElement("div");
    legend.className = "mini-chart-legend";
    buildMiniLegendItems(datasetSeries).forEach((item) => legend.appendChild(item));

    const miniCanvas = document.createElement("canvas");
    miniCanvas.width = 420;
    miniCanvas.height = 260;

    card.append(title, sub, badge, legend, miniCanvas);
    dom.smallMultiplesGrid.appendChild(card);
    drawMiniChart(miniCanvas, datasetSeries);
  });
}

function drawMiniChart(canvas, seriesGroup) {
  const miniCtx = canvas.getContext("2d");
  const width = canvas.width;
  const height = canvas.height;
  const padding = { top: 24, right: 18, bottom: 34, left: 48 };
  const plotWidth = width - padding.left - padding.right;
  const plotHeight = height - padding.top - padding.bottom;
  const xValues = seriesGroup.flatMap((series) => series.rows.map((point) => point.x));
  const yValues = seriesGroup.flatMap((series) => series.rows.map((point) => point.y));
  const xDomain = pickDomain(xValues, state.ranges.xMin, state.ranges.xMax);
  const yDomain = pickDomain(yValues, state.ranges.yMin, state.ranges.yMax);

  miniCtx.clearRect(0, 0, width, height);
  const bg = miniCtx.createLinearGradient(0, 0, 0, height);
  bg.addColorStop(0, "rgba(255,255,255,0.95)");
  bg.addColorStop(1, "rgba(249,244,236,0.98)");
  miniCtx.fillStyle = bg;
  miniCtx.fillRect(0, 0, width, height);

  miniCtx.strokeStyle = "rgba(31,23,17,0.12)";
  miniCtx.lineWidth = 1;
  miniCtx.strokeRect(padding.left, padding.top, plotWidth, plotHeight);

  seriesGroup.forEach((series) => {
    miniCtx.strokeStyle = series.color;
    miniCtx.lineWidth = series.axis === "y2" ? 1.8 : 2.4;
    miniCtx.setLineDash(series.lineDash);
    miniCtx.beginPath();
    let penDown = false;
    series.rows.forEach((point) => {
      const x = mapValue(point.x, xDomain.min, xDomain.max, padding.left, padding.left + plotWidth);
      const y = mapValue(point.y, yDomain.min, yDomain.max, padding.top + plotHeight, padding.top);
      if (!penDown) {
        miniCtx.moveTo(x, y);
        penDown = true;
      } else {
        miniCtx.lineTo(x, y);
      }
      if (point.breakAfter) {
        penDown = false;
      }
    });
    miniCtx.stroke();
  });
  miniCtx.setLineDash([]);

  miniCtx.fillStyle = "#6f6258";
  miniCtx.font = "12px Segoe UI";
  miniCtx.textAlign = "left";
  miniCtx.fillText(`max ${formatNumber(Math.max(...yValues))}`, 12, 18);
  miniCtx.fillText(`min ${formatNumber(Math.min(...yValues))}`, 12, height - 10);
}

function buildMiniLegendItems(seriesGroup) {
  return seriesGroup.map((series) => {
    const item = document.createElement("div");
    item.className = "mini-chart-legend-item";
    item.style.color = series.axis === "y2" ? withAlpha(series.color, 0.72) : series.color;

    const line = document.createElement("span");
    line.className = "mini-chart-legend-line";
    if (series.lineDash.length) {
      line.style.borderTopStyle = "dashed";
    }

    const text = document.createElement("span");
    text.textContent = getMiniLegendLabel(series);

    item.append(line, text);
    return item;
  });
}

function getMiniLegendLabel(series) {
  if (series.axis === "y2") {
    return "目标转速";
  }

  const column = String(series.column || "").toLowerCase();
  if (column.includes("raw") && column.includes("speed")) {
    return "原始速度";
  }
  if (column.includes("final") && column.includes("speed")) {
    return state.analysisMode === "pll" ? "控制使用速度（辅助）" : "控制使用速度";
  }
  if (column.includes("filtered") && column.includes("speed")) {
    return state.analysisMode === "pll" ? "处理后速度（辅助）" : "滤波速度";
  }
  return translateColumnLabel(series.column);
}

function groupSeriesByDataset(seriesList) {
  const grouped = new Map();

  seriesList.forEach((series) => {
    if (!grouped.has(series.datasetId)) {
      grouped.set(series.datasetId, []);
    }
    grouped.get(series.datasetId).push(series);
  });

  return Array.from(grouped.values());
}
