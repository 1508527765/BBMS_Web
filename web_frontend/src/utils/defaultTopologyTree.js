export const BBMS_TOPOLOGY_STORAGE_KEY = "bbms_topology_config_v1";

/**
 * 仅返回 localStorage 中已有拓扑；不再自动生成默认/模拟拓扑。
 * @returns {any[]}
 */
export function ensureTopologyInStorage() {
  const key = BBMS_TOPOLOGY_STORAGE_KEY;
  try {
    const raw = localStorage.getItem(key);
    if (raw) {
      const parsed = JSON.parse(raw);
      if (Array.isArray(parsed) && parsed.length) return parsed;
    }
  } catch (e) {
    return [];
  }
  return [];
}
