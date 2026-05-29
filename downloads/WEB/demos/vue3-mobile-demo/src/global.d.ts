declare module "imou-player";

// 声明模块
declare module "*/vconsole.js" {
  export function initVConsole(): void;
  export function destroyVConsole(): void;
  export function getVConsole(): any;
}

declare module "*/vconsole.min.js" {
  // VConsole is available globally via window.VConsole
}

declare module "*/vconsole-log-export-plugin.js" {
  export default class VConsoleLogExportPlugin {
    constructor();
  }
}

// Global VConsole type declaration
declare global {
  interface Window {
    VConsole: any;
  }
}
