/**
 * Robot 3D Visualization System
 * Advanced Three.js-based 3D viewer for robot models with real-time state updates
 */

class Robot3DViewer {
    constructor(containerId, options = {}) {
        this.containerId = containerId;
        this.container = document.getElementById(containerId);
        
        // Configuration options
        this.options = {
            width: options.width || this.container.clientWidth || 600,
            height: options.height || this.container.clientHeight || 400,
            backgroundColor: options.backgroundColor || 0x1a1a2e,
            cameraPosition: options.cameraPosition || { x: 5, y: 5, z: 5 },
            enableControls: options.enableControls !== false,
            showGrid: options.showGrid !== false,
            showAxes: options.showAxes !== false,
            modelScale: options.modelScale || 1.0,
            animationSpeed: options.animationSpeed || 1.0,
            ...options
        };
        
        // Three.js components
        this.scene = null;
        this.camera = null;
        this.renderer = null;
        this.controls = null;
        this.loader = null;
        
        // Robot components
        this.robotGroup = null;
        this.baseJoint = null;
        this.armJoints = [];
        this.gripper = null;
        this.targetMarker = null;
        
        // Animation system
        this.animationMixer = null;
        this.clock = new THREE.Clock();
        this.isAnimating = false;
        
        // Robot state
        this.currentPosition = { x: 0, y: 0, z: 0 };
        this.targetPosition = { x: 0, y: 0, z: 0 };
        this.jointAngles = { base: 0, shoulder: 0, elbow: 0, wrist: 0 };
        this.gripperState = false;
        
        this.init();
    }
    
    init() {
        this.createScene();
        this.createCamera();
        this.createRenderer();
        this.createLights();
        this.createControls();
        this.createEnvironment();
        this.createDemoRobot();
        this.setupEventListeners();
        this.animate();
        
        console.log('Robot 3D Viewer initialized successfully');
    }
    
    createScene() {
        this.scene = new THREE.Scene();
        this.scene.background = new THREE.Color(this.options.backgroundColor);
        this.scene.fog = new THREE.Fog(this.options.backgroundColor, 10, 50);
    }
    
    createCamera() {
        const aspect = this.options.width / this.options.height;
        this.camera = new THREE.PerspectiveCamera(75, aspect, 0.1, 1000);
        
        const pos = this.options.cameraPosition;
        this.camera.position.set(pos.x, pos.y, pos.z);
        this.camera.lookAt(0, 0, 0);
    }
    
    createRenderer() {
        this.renderer = new THREE.WebGLRenderer({ 
            antialias: true,
            alpha: true,
            preserveDrawingBuffer: true
        });
        
        this.renderer.setSize(this.options.width, this.options.height);
        this.renderer.setPixelRatio(window.devicePixelRatio);
        this.renderer.shadowMap.enabled = true;
        this.renderer.shadowMap.type = THREE.PCFSoftShadowMap;
        this.renderer.outputEncoding = THREE.sRGBEncoding;
        this.renderer.toneMapping = THREE.ACESFilmicToneMapping;
        this.renderer.toneMappingExposure = 1.0;
        
        this.container.appendChild(this.renderer.domElement);
    }
    
    createLights() {
        // Ambient light for overall illumination
        const ambientLight = new THREE.AmbientLight(0x404040, 0.4);
        this.scene.add(ambientLight);
        
        // Main directional light (sun)
        const directionalLight = new THREE.DirectionalLight(0xffffff, 0.8);
        directionalLight.position.set(10, 10, 5);
        directionalLight.castShadow = true;
        directionalLight.shadow.mapSize.width = 2048;
        directionalLight.shadow.mapSize.height = 2048;
        directionalLight.shadow.camera.near = 0.5;
        directionalLight.shadow.camera.far = 50;
        directionalLight.shadow.camera.left = -10;
        directionalLight.shadow.camera.right = 10;
        directionalLight.shadow.camera.top = 10;
        directionalLight.shadow.camera.bottom = -10;
        this.scene.add(directionalLight);
        
        // Fill light
        const fillLight = new THREE.DirectionalLight(0x87ceeb, 0.3);
        fillLight.position.set(-5, 5, -5);
        this.scene.add(fillLight);
        
        // Rim light
        const rimLight = new THREE.DirectionalLight(0xffffff, 0.2);
        rimLight.position.set(0, -10, 0);
        this.scene.add(rimLight);
    }
    
    createControls() {
        if (this.options.enableControls && typeof THREE.OrbitControls !== 'undefined') {
            this.controls = new THREE.OrbitControls(this.camera, this.renderer.domElement);
            this.controls.enableDamping = true;
            this.controls.dampingFactor = 0.05;
            this.controls.maxPolarAngle = Math.PI;
            this.controls.minDistance = 2;
            this.controls.maxDistance = 20;
        }
    }
    
    createEnvironment() {
        // Ground plane
        const groundGeometry = new THREE.PlaneGeometry(20, 20);
        const groundMaterial = new THREE.MeshLambertMaterial({ 
            color: 0x2a2a3e,
            transparent: true,
            opacity: 0.8
        });
        const ground = new THREE.Mesh(groundGeometry, groundMaterial);
        ground.rotation.x = -Math.PI / 2;
        ground.receiveShadow = true;
        this.scene.add(ground);
        
        // Grid helper
        if (this.options.showGrid) {
            const gridHelper = new THREE.GridHelper(20, 20, 0x444444, 0x333333);
            gridHelper.position.y = 0.01; // Slightly above ground
            this.scene.add(gridHelper);
        }
        
        // Axes helper
        if (this.options.showAxes) {
            const axesHelper = new THREE.AxesHelper(2);
            this.scene.add(axesHelper);
        }
        
        // Work envelope (transparent sphere)
        const envelopeGeometry = new THREE.SphereGeometry(3, 32, 32);
        const envelopeMaterial = new THREE.MeshBasicMaterial({
            color: 0x00ff00,
            transparent: true,
            opacity: 0.1,
            wireframe: true
        });
        const envelope = new THREE.Mesh(envelopeGeometry, envelopeMaterial);
        envelope.position.y = 1.5;
        this.scene.add(envelope);
    }
    
    createDemoRobot() {
        this.robotGroup = new THREE.Group();
        this.robotGroup.scale.setScalar(this.options.modelScale);
        
        // Robot base
        const baseGeometry = new THREE.CylinderGeometry(0.5, 0.6, 0.3, 16);
        const baseMaterial = new THREE.MeshPhongMaterial({ 
            color: 0x2c3e50,
            shininess: 100
        });
        const base = new THREE.Mesh(baseGeometry, baseMaterial);
        base.castShadow = true;
        base.receiveShadow = true;
        this.robotGroup.add(base);
        
        // Base joint (rotates around Y-axis)
        this.baseJoint = new THREE.Group();
        this.baseJoint.position.y = 0.15;
        this.robotGroup.add(this.baseJoint);
        
        // Link 1 (shoulder)
        const link1Geometry = new THREE.BoxGeometry(0.3, 0.8, 0.2);
        const link1Material = new THREE.MeshPhongMaterial({ 
            color: 0x3498db,
            shininess: 100
        });
        const link1 = new THREE.Mesh(link1Geometry, link1Material);
        link1.position.y = 0.4;
        link1.castShadow = true;
        link1.receiveShadow = true;
        this.baseJoint.add(link1);
        
        // Shoulder joint
        const shoulderJoint = new THREE.Group();
        shoulderJoint.position.set(0, 0.8, 0);
        this.baseJoint.add(shoulderJoint);
        this.armJoints.push(shoulderJoint);
        
        // Link 2 (upper arm)
        const link2Geometry = new THREE.BoxGeometry(0.2, 1.5, 0.15);
        const link2Material = new THREE.MeshPhongMaterial({ 
            color: 0xe74c3c,
            shininess: 100
        });
        const link2 = new THREE.Mesh(link2Geometry, link2Material);
        link2.position.y = 0.75;
        link2.castShadow = true;
        link2.receiveShadow = true;
        shoulderJoint.add(link2);
        
        // Elbow joint
        const elbowJoint = new THREE.Group();
        elbowJoint.position.set(0, 1.5, 0);
        shoulderJoint.add(elbowJoint);
        this.armJoints.push(elbowJoint);
        
        // Link 3 (forearm)
        const link3Geometry = new THREE.BoxGeometry(0.15, 1.2, 0.12);
        const link3Material = new THREE.MeshPhongMaterial({ 
            color: 0xf39c12,
            shininess: 100
        });
        const link3 = new THREE.Mesh(link3Geometry, link3Material);
        link3.position.y = 0.6;
        link3.castShadow = true;
        link3.receiveShadow = true;
        elbowJoint.add(link3);
        
        // Wrist joint
        const wristJoint = new THREE.Group();
        wristJoint.position.set(0, 1.2, 0);
        elbowJoint.add(wristJoint);
        this.armJoints.push(wristJoint);
        
        // End effector/gripper
        this.gripper = new THREE.Group();
        this.gripper.position.y = 0.2;
        wristJoint.add(this.gripper);
        
        // Gripper base
        const gripperBaseGeometry = new THREE.CylinderGeometry(0.08, 0.08, 0.15, 8);
        const gripperBaseMaterial = new THREE.MeshPhongMaterial({ 
            color: 0x34495e,
            shininess: 100
        });
        const gripperBase = new THREE.Mesh(gripperBaseGeometry, gripperBaseMaterial);
        gripperBase.castShadow = true;
        this.gripper.add(gripperBase);
        
        // Gripper fingers
        const fingerGeometry = new THREE.BoxGeometry(0.03, 0.15, 0.08);
        const fingerMaterial = new THREE.MeshPhongMaterial({ 
            color: 0x7f8c8d,
            shininess: 100
        });
        
        this.leftFinger = new THREE.Mesh(fingerGeometry, fingerMaterial);
        this.leftFinger.position.set(-0.05, 0.1, 0);
        this.leftFinger.castShadow = true;
        this.gripper.add(this.leftFinger);
        
        this.rightFinger = new THREE.Mesh(fingerGeometry, fingerMaterial);
        this.rightFinger.position.set(0.05, 0.1, 0);
        this.rightFinger.castShadow = true;
        this.gripper.add(this.rightFinger);
        
        // Target position marker
        this.createTargetMarker();
        
        this.scene.add(this.robotGroup);
        console.log('Demo robot model created successfully');
    }
    
    createTargetMarker() {
        this.targetMarker = new THREE.Group();
        
        // Target sphere
        const targetGeometry = new THREE.SphereGeometry(0.05, 16, 16);
        const targetMaterial = new THREE.MeshBasicMaterial({ 
            color: 0xff0000,
            transparent: true,
            opacity: 0.7
        });
        const targetSphere = new THREE.Mesh(targetGeometry, targetMaterial);
        this.targetMarker.add(targetSphere);
        
        // Target crosshairs
        const crosshairMaterial = new THREE.LineBasicMaterial({ color: 0xff0000 });
        
        const xLineGeometry = new THREE.BufferGeometry().setFromPoints([
            new THREE.Vector3(-0.1, 0, 0),
            new THREE.Vector3(0.1, 0, 0)
        ]);
        const xLine = new THREE.Line(xLineGeometry, crosshairMaterial);
        this.targetMarker.add(xLine);
        
        const yLineGeometry = new THREE.BufferGeometry().setFromPoints([
            new THREE.Vector3(0, -0.1, 0),
            new THREE.Vector3(0, 0.1, 0)
        ]);
        const yLine = new THREE.Line(yLineGeometry, crosshairMaterial);
        this.targetMarker.add(yLine);
        
        const zLineGeometry = new THREE.BufferGeometry().setFromPoints([
            new THREE.Vector3(0, 0, -0.1),
            new THREE.Vector3(0, 0, 0.1)
        ]);
        const zLine = new THREE.Line(zLineGeometry, crosshairMaterial);
        this.targetMarker.add(zLine);
        
        this.targetMarker.visible = false;
        this.scene.add(this.targetMarker);
    }
    
    setupEventListeners() {
        // Handle window resize
        window.addEventListener('resize', () => this.handleResize());
        
        // Handle container resize
        if (window.ResizeObserver) {
            const resizeObserver = new ResizeObserver(() => this.handleResize());
            resizeObserver.observe(this.container);
        }
    }
    
    handleResize() {
        const width = this.container.clientWidth;
        const height = this.container.clientHeight;
        
        this.camera.aspect = width / height;
        this.camera.updateProjectionMatrix();
        
        this.renderer.setSize(width, height);
    }
    
    animate() {
        requestAnimationFrame(() => this.animate());
        
        const delta = this.clock.getDelta();
        
        if (this.controls) {
            this.controls.update();
        }
        
        if (this.animationMixer) {
            this.animationMixer.update(delta);
        }
        
        this.updateRobotAnimation(delta);
        this.renderer.render(this.scene, this.camera);
    }
    
    updateRobotAnimation(delta) {
        if (!this.isAnimating) return;
        
        // Animate to target position using inverse kinematics (simplified)
        const targetDistance = new THREE.Vector3()
            .subVectors(
                new THREE.Vector3(this.targetPosition.x, this.targetPosition.y, this.targetPosition.z),
                new THREE.Vector3(this.currentPosition.x, this.currentPosition.y, this.currentPosition.z)
            ).length();
        
        if (targetDistance > 0.01) {
            // Lerp towards target position
            const lerpFactor = Math.min(delta * this.options.animationSpeed, 1.0);
            this.currentPosition.x += (this.targetPosition.x - this.currentPosition.x) * lerpFactor;
            this.currentPosition.y += (this.targetPosition.y - this.currentPosition.y) * lerpFactor;
            this.currentPosition.z += (this.targetPosition.z - this.currentPosition.z) * lerpFactor;
            
            this.updateInverseKinematics();
        } else {
            this.isAnimating = false;
        }
    }
    
    updateInverseKinematics() {
        // Simplified inverse kinematics for demonstration
        const target = new THREE.Vector3(this.currentPosition.x, this.currentPosition.y, this.currentPosition.z);
        const distance = target.length();
        
        // Calculate joint angles (simplified)
        this.jointAngles.base = Math.atan2(target.x, target.z);
        this.jointAngles.shoulder = Math.atan2(target.y - 0.8, Math.sqrt(target.x * target.x + target.z * target.z));
        this.jointAngles.elbow = Math.PI - Math.acos(Math.max(-1, Math.min(1, distance / 3)));
        
        // Apply rotations to joints
        if (this.baseJoint) {
            this.baseJoint.rotation.y = this.jointAngles.base;
        }
        
        if (this.armJoints[0]) {
            this.armJoints[0].rotation.z = this.jointAngles.shoulder;
        }
        
        if (this.armJoints[1]) {
            this.armJoints[1].rotation.z = this.jointAngles.elbow;
        }
        
        // Update gripper
        this.updateGripper();
    }
    
    updateGripper() {
        if (!this.leftFinger || !this.rightFinger) return;
        
        const fingerOffset = this.gripperState ? 0.02 : 0.08;
        this.leftFinger.position.x = -fingerOffset;
        this.rightFinger.position.x = fingerOffset;
    }
    
    // Public API methods
    moveToPosition(x, y, z, animate = true) {
        this.targetPosition = { x, y, z };
        
        // Update target marker
        if (this.targetMarker) {
            this.targetMarker.position.set(x, y, z);
            this.targetMarker.visible = true;
            
            // Hide marker after animation
            setTimeout(() => {
                if (this.targetMarker) this.targetMarker.visible = false;
            }, 3000);
        }
        
        if (animate) {
            this.isAnimating = true;
        } else {
            this.currentPosition = { x, y, z };
            this.updateInverseKinematics();
        }
    }
    
    setGripperState(open) {
        this.gripperState = open;
        this.updateGripper();
    }
    
    loadModel(url, onLoad, onProgress, onError) {
        if (!this.loader) {
            // Try to determine loader based on file extension
            const ext = url.split('.').pop().toLowerCase();
            
            if (ext === 'gltf' || ext === 'glb') {
                this.loader = new THREE.GLTFLoader();
            } else if (ext === 'obj') {
                this.loader = new THREE.OBJLoader();
            } else if (ext === 'stl') {
                this.loader = new THREE.STLLoader();
            } else if (ext === 'ply') {
                this.loader = new THREE.PLYLoader();
            } else {
                console.warn(`Unsupported file format: ${ext}`);
                if (onError) onError(new Error(`Unsupported file format: ${ext}`));
                return;
            }
        }
        
        this.loader.load(
            url,
            (model) => {
                // Remove existing robot if any
                if (this.robotGroup) {
                    this.scene.remove(this.robotGroup);
                }
                
                // Process loaded model
                let mesh;
                if (model.scene) {
                    // GLTF format
                    mesh = model.scene;
                } else if (model.isBufferGeometry) {
                    // STL/PLY format
                    const material = new THREE.MeshPhongMaterial({ color: 0x3498db });
                    mesh = new THREE.Mesh(model, material);
                } else {
                    // OBJ format
                    mesh = model;
                }
                
                mesh.scale.setScalar(this.options.modelScale);
                mesh.traverse((child) => {
                    if (child.isMesh) {
                        child.castShadow = true;
                        child.receiveShadow = true;
                    }
                });
                
                this.robotGroup = mesh;
                this.scene.add(this.robotGroup);
                
                if (onLoad) onLoad(mesh);
                console.log('3D model loaded successfully');
            },
            onProgress,
            onError
        );
    }
    
    setRobotState(state) {
        if (state.position) {
            this.moveToPosition(state.position.x, state.position.y, state.position.z, true);
        }
        
        if (typeof state.gripper_open === 'boolean') {
            this.setGripperState(state.gripper_open);
        }
    }
    
    resetView() {
        if (this.controls) {
            this.controls.reset();
        }
        
        const pos = this.options.cameraPosition;
        this.camera.position.set(pos.x, pos.y, pos.z);
        this.camera.lookAt(0, 0, 0);
    }
    
    dispose() {
        if (this.controls) {
            this.controls.dispose();
        }
        
        if (this.renderer) {
            this.renderer.dispose();
        }
        
        // Clean up geometries and materials
        this.scene.traverse((object) => {
            if (object.geometry) {
                object.geometry.dispose();
            }
            if (object.material) {
                if (Array.isArray(object.material)) {
                    object.material.forEach(material => material.dispose());
                } else {
                    object.material.dispose();
                }
            }
        });
    }
}

// Export for use in other modules
window.Robot3DViewer = Robot3DViewer;