/**
 * 首页拓扑展示：架构行划分 + RBMS 下合成 PBMS / 电芯 / 温度模块。
 */

export const TOPO_PBMS_PER_RBMS = 8;
export const TOPO_CELLS_PER_PBMS = 52;
export const TOPO_TEMPS_PER_PBMS = 29;

export const STACK_BMS_TYPE = 1;
export const RBMS_DEVICE_TYPE = 2;

function inferVirtualStackFromLabel(node) {
  const s = String((node && node.label) || "").toLowerCase();
  if (/virtual\s*bbms|虚拟\s*bbms/.test(s)) return true;
  if (s.includes("bbms") && !s.includes("rbms")) return true;
  return false;
}

export function isRealStackBms(node) {
  if (!node || node.type !== "device") return false;
  return Number(node.deviceType) === STACK_BMS_TYPE;
}

export function isVirtualStackBms(node) {
  if (!node || node.type !== "virtual") return false;
  const vdt = node.virtualDevType;
  if (vdt !== undefined && vdt !== null && vdt !== "") {
    return Number(vdt) === STACK_BMS_TYPE;
  }
  return inferVirtualStackFromLabel(node);
}

export function isRbmsNode(node) {
  if (!node) return false;
  return Number(node.deviceType) === RBMS_DEVICE_TYPE;
}

/** 首页不展示本机根：取 Local system 的直接子节点，每子节点为一行架构 */
export function buildArchitectureRows(roots) {
  const list = Array.isArray(roots) ? roots : [];
  const localRoot = list.find((n) => n && n.type === "device" && Number(n.deviceType) === 0);
  const kids = (localRoot && localRoot.children) || [];
  const children = Array.isArray(kids) ? kids : [];

  const realRows = [];
  const otherRows = [];
  const virtualRows = [];

  children.forEach((node) => {
    const rowChildren = Array.isArray(node.children) ? node.children : [];
    const row = {
      bbms: node,
      children: rowChildren,
      isVirtual: false,
    };
    if (isRealStackBms(node)) {
      realRows.push(row);
    } else if (isVirtualStackBms(node)) {
      row.isVirtual = true;
      virtualRows.push(row);
    } else {
      otherRows.push(row);
    }
  });

  return [...realRows, ...otherRows, ...virtualRows];
}

/** 仅真实设备节点参与在线状态轮询 */
export function collectRealDeviceNodes(architectureRow) {
  const out = [];
  const seen = new Set();
  const walk = (n) => {
    if (!n || n.type === "synthetic") return;
    if (n.type === "device") {
      const key = String(n.id);
      if (!seen.has(key)) {
        seen.add(key);
        out.push(n);
      }
    }
    (n.children || []).forEach(walk);
  };
  if (architectureRow && architectureRow.bbms) walk(architectureRow.bbms);
  (architectureRow && architectureRow.children || []).forEach(walk);
  return out;
}

export function architectureSummary(row) {
  const kids = (row && row.children) || [];
  let rbms = 0;
  let aux = 0;
  kids.forEach((ch) => {
    if (isRbmsNode(ch)) rbms += 1;
    else aux += 1;
  });
  return { rbms, aux, total: kids.length };
}
