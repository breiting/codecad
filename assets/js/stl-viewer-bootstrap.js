// Load the module relative to THIS file, not the page URL
(() => {
  const here = document.currentScript?.src || "";
  const modUrl = new URL("./stl-viewer.module.js", here).href;
  import(modUrl);
})();
