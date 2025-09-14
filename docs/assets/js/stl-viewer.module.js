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

  const scene = new THREE.Scene();
  scene.background = new THREE.Color(bgHex);

  const camera = new THREE.PerspectiveCamera(
    50,
    el.clientWidth / el.clientHeight,
    0.1,
    2000,
  );
  camera.position.set(120, 90, 120);

  const renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer.setPixelRatio(window.devicePixelRatio);
  renderer.setSize(el.clientWidth, el.clientHeight);
  el.appendChild(renderer.domElement);

  const controls = new OrbitControls(camera, renderer.domElement);
  controls.enableDamping = true;
  controls.autoRotate = autoRotate;

  scene.add(new THREE.HemisphereLight(0xffffff, 0x444444, 0.7));
  const dir = new THREE.DirectionalLight(0xffffff, 0.7);
  dir.position.set(1, 1, 1);
  scene.add(dir);

  let grid;
  if (showGrid) {
    grid = new THREE.GridHelper(200, 20, 0x888888, 0xdddddd);
    grid.position.y = -0.01;
    scene.add(grid);
  }

  const loader = new STLLoader();
  let mesh;
  loader.load(src, (geometry) => {
    const material = new THREE.MeshPhongMaterial({
      color: new THREE.Color(colorHex),
      specular: 0x111111,
      shininess: 60,
    });
    mesh = new THREE.Mesh(geometry, material);

    geometry.computeBoundingBox();
    const bb = geometry.boundingBox;
    const size = new THREE.Vector3().subVectors(bb.max, bb.min);
    const center = new THREE.Vector3()
      .addVectors(bb.min, bb.max)
      .multiplyScalar(0.5);

    mesh.position.sub(center);
    const maxDim = Math.max(size.x, size.y, size.z) || 1;
    mesh.scale.setScalar(100 / maxDim);
    scene.add(mesh);

    const dist = 2.2 * 100;
    camera.position.set(dist, dist * 0.75, dist);
    controls.target.set(0, 0, 0);
  });

  const ro = new ResizeObserver(() => {
    const w = el.clientWidth || 1,
      h = el.clientHeight || 1;
    camera.aspect = w / h;
    camera.updateProjectionMatrix();
    renderer.setSize(w, h);
  });
  ro.observe(el);

  let disposed = false;
  (function animate() {
    if (disposed) return;
    requestAnimationFrame(animate);
    controls.update();
    renderer.render(scene, camera);
  })();

  return () => {
    disposed = true;
    ro.disconnect();
    controls.dispose();
    renderer.dispose();
    if (mesh) {
      mesh.geometry.dispose();
      if (mesh.material?.dispose) mesh.material.dispose();
    }
    if (grid) grid.geometry.dispose();
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

// Re-init on MkDocs Material SPA page changes
if (window.document$?.subscribe) {
  window.document$.subscribe(() => {
    document.querySelectorAll(".stl-viewer").forEach((el) => el.__dispose?.());
    initAll();
  });
}
