import * as THREE from "https://esm.sh/three@0.160.0";
import { OrbitControls } from "https://esm.sh/three@0.160.0/examples/jsm/controls/OrbitControls.js";
import { STLLoader } from "https://esm.sh/three@0.160.0/examples/jsm/loaders/STLLoader.js";

function parseBool(v, d = false) {
  return v == null ? d : String(v).toLowerCase() === "true";
}

function createViewer(el) {
  const src = el.getAttribute("data-src");
  const colorHex = el.getAttribute("data-color") || "#4b9fea";
  const bgHex = el.getAttribute("data-bg") || "#1e2129";
  const showGrid = parseBool(el.getAttribute("data-grid"), true);
  const autoRotate = parseBool(el.getAttribute("data-autorotate"), false);
  const showControls = parseBool(el.getAttribute("data-controls"), false);

  // Sizing helpers (robust against 0×0)
  const getSize = () => {
    const r = el.getBoundingClientRect();
    const w = Math.floor(r.width) || el.clientWidth || 800;
    const h = Math.floor(r.height) || el.clientHeight || 500;
    return { w, h };
  };
  let { w, h } = getSize();

  // Scene
  const scene = new THREE.Scene();
  scene.background = new THREE.Color(bgHex);

  // Camera
  const camera = new THREE.PerspectiveCamera(50, w / h, 0.1, 2000);
  camera.position.set(120, 90, 120);

  // Renderer
  const renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer.setPixelRatio(window.devicePixelRatio);
  renderer.setSize(w, h);
  el.appendChild(renderer.domElement);

  // Controls
  const controls = new OrbitControls(camera, renderer.domElement);
  controls.enableDamping = true;
  controls.autoRotate = autoRotate;

  // Lights
  scene.add(new THREE.HemisphereLight(0xffffff, 0x444444, 0.7));
  const dir = new THREE.DirectionalLight(0xffffff, 0.7);
  dir.position.set(1, 1, 1);
  scene.add(dir);

  // Axis
  const axesHelper = new THREE.AxesHelper(100);
  axesHelper.rotateX(-Math.PI / 2);
  scene.add(axesHelper);

  // Grid
  let grid;
  if (showGrid) {
    grid = new THREE.GridHelper(200, 20, 0x888888, 0xdddddd);
    grid.position.y = -0.01;
    scene.add(grid);
  }

  // Load STL
  const loader = new STLLoader();
  let mesh;
  loader.load(src, (geometry) => {
    const material = new THREE.MeshPhongMaterial({
      color: new THREE.Color(colorHex),
      specular: 0x111111,
      shininess: 60,
    });
    mesh = new THREE.Mesh(geometry, material);
    mesh.rotateX(-Math.PI / 2);

    mesh.scale.setScalar(2.0);
    scene.add(mesh);

    // frame
    const dist = 2.2 * 20;
    camera.position.set(dist, dist * 0.75, dist);
    controls.target.set(0, 0, 0);
  });

  // Resize
  const ro = new ResizeObserver(() => {
    const { w, h } = getSize();
    camera.aspect = w / h;
    camera.updateProjectionMatrix();
    renderer.setSize(w, h);
  });
  ro.observe(el);
  // force one update
  (() => {
    const { w, h } = getSize();
    camera.aspect = w / h;
    camera.updateProjectionMatrix();
    renderer.setSize(w, h);
  })();

  // Toolbar (optional)
  let toolbar, cbRotate, cbGrid;
  if (showControls) {
    toolbar = document.createElement("div");
    toolbar.className = "stl-toolbar";

    // autorotate
    const labelRot = document.createElement("label");
    cbRotate = document.createElement("input");
    cbRotate.type = "checkbox";
    cbRotate.checked = autoRotate;
    labelRot.appendChild(cbRotate);
    labelRot.appendChild(document.createTextNode("Auto-rotate"));

    // grid
    const labelGrid = document.createElement("label");
    cbGrid = document.createElement("input");
    cbGrid.type = "checkbox";
    cbGrid.checked = showGrid;
    labelGrid.appendChild(cbGrid);
    labelGrid.appendChild(document.createTextNode("Grid"));

    toolbar.appendChild(labelRot);
    toolbar.appendChild(labelGrid);
    el.appendChild(toolbar);

    // events
    cbRotate.addEventListener("change", () => {
      controls.autoRotate = cbRotate.checked;
    });
    cbGrid.addEventListener("change", () => {
      if (cbGrid.checked) {
        if (!grid) {
          grid = new THREE.GridHelper(200, 20, 0x888888, 0xdddddd);
          grid.position.y = -0.01;
        }
        scene.add(grid);
      } else if (grid) {
        scene.remove(grid);
      }
    });
  }

  // Render loop
  let disposed = false;
  (function animate() {
    if (disposed) return;
    requestAnimationFrame(animate);
    controls.update();
    renderer.render(scene, camera);
  })();

  // Disposer for SPA nav
  return () => {
    disposed = true;
    ro.disconnect();
    controls.dispose();
    renderer.dispose();
    if (mesh) {
      mesh.geometry.dispose();
      if (mesh.material?.dispose) mesh.material.dispose();
    }
    if (grid) grid.geometry?.dispose?.();
    while (el.firstChild) el.removeChild(el.firstChild);
  };
}

function initAll() {
  document.querySelectorAll(".stl-viewer").forEach((el) => {
    if (!el.__stlReady) {
      el.__stlReady = true;
      el.__dispose = createViewer(el);
    }
  });
}

if (document.readyState === "loading") {
  document.addEventListener("DOMContentLoaded", initAll);
} else {
  initAll();
}

if (window.document$?.subscribe) {
  window.document$.subscribe(() => {
    document.querySelectorAll(".stl-viewer").forEach((el) => el.__dispose?.());
    initAll();
  });
}
