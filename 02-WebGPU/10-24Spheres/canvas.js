// Global variables
var canvas = null;
var canvas_original_width;
var canvas_original_height;
var bFullScreen = false;

// Web GPU related variables
var clear_color;
let device = null;
let context = null;
let queue = null;
let canvas_format = null;
let animation_frame_id = null;

let render_pipeline = null;
let buffer_uniform = null;  // Now it will contains light, material and mvp matrices
let bindingGroups_uniform = null;
let buffer_material = null; // Now it will contains materials
let bindingGroups_material = null;

var lightAmbient = new Float32Array([0.1, 0.1, 0.1, 0.0]);
var lightDiffuse = new Float32Array([1.0, 1.0, 1.0, 0.0]);
var lightSpecular = new Float32Array([1.0, 1.0, 1.0, 0.0]);

var lightPosition = new Float32Array([100.0, 100.0, 100.0, 1.0]);

var lKeyPressed = new Uint32Array([0,0,0,0]); // we are using x component only for 1 = L key pressed, 0 = L key not pressed
var isLightingEnabled = false; // we are using x component only for 1 = lighting enabled, 0 = lighting disabled

var accessChoosen = " ";
var lightAngle = 0.0;

var materialTable =
[
    // row 1
    { name: "emerald",        ambient: [0.0215,    0.1745,    0.0215,    1.0], diffuse: [0.07568,    0.61424,    0.07568,    1.0], specular: [0.633,    0.727811, 0.633,    1.0], shininess: 0.6        * 128.0 },
    { name: "brass",          ambient: [0.329412,  0.223529,  0.027451,  1.0], diffuse: [0.780392,   0.568627,   0.113725,   1.0], specular: [0.992157, 0.941176, 0.807843, 1.0], shininess: 0.21794872 * 128.0 },
    { name: "black plastic",  ambient: [0.0,       0.0,       0.0,       1.0], diffuse: [0.01,       0.01,       0.01,       1.0], specular: [0.5,      0.5,      0.5,      1.0], shininess: 0.25       * 128.0 },
    { name: "black rubber",   ambient: [0.02,      0.02,      0.02,      1.0], diffuse: [0.01,       0.01,       0.01,       1.0], specular: [0.4,      0.4,      0.4,      1.0], shininess: 0.78125    * 128.0 },
    // row 2
    { name: "jade",           ambient: [0.135,     0.2225,    0.1575,    1.0], diffuse: [0.54,       0.89,       0.63,       1.0], specular: [0.316228, 0.316228, 0.316228, 1.0], shininess: 0.1        * 128.0 },
    { name: "bronze",         ambient: [0.2125,    0.1275,    0.054,     1.0], diffuse: [0.714,      0.4284,     0.18144,    1.0], specular: [0.393548, 0.271906, 0.166721, 1.0], shininess: 0.2        * 128.0 },
    { name: "cyan plastic",   ambient: [0.0,       0.1,       0.06,      1.0], diffuse: [0.0,        0.50980392, 0.50980392, 1.0], specular: [0.50196078, 0.50196078, 0.50196078, 1.0], shininess: 0.25 * 128.0 },
    { name: "cyan rubber",    ambient: [0.0,       0.05,      0.05,      1.0], diffuse: [0.4,        0.5,        0.5,        1.0], specular: [0.04,     0.7,      0.7,      1.0], shininess: 0.078125   * 128.0 },
    // row 3
    { name: "obsidian",       ambient: [0.05375,   0.05,      0.06625,   1.0], diffuse: [0.18275,    0.17,       0.22525,    1.0], specular: [0.332741, 0.328634, 0.346435, 1.0], shininess: 0.3        * 128.0 },
    { name: "chrome",         ambient: [0.25,      0.25,      0.25,      1.0], diffuse: [0.4,        0.4,        0.4,        1.0], specular: [0.774597, 0.774597, 0.774597, 1.0], shininess: 0.6        * 128.0 },
    { name: "green plastic",  ambient: [0.0,       0.0,       0.0,       1.0], diffuse: [0.1,        0.35,       0.1,        1.0], specular: [0.45,     0.55,     0.45,     1.0], shininess: 0.25       * 128.0 },
    { name: "green rubber",   ambient: [0.0,       0.05,      0.0,       1.0], diffuse: [0.4,        0.5,        0.4,        1.0], specular: [0.04,     0.7,      0.04,     1.0], shininess: 0.0788125  * 128.0 },
    // row 4
    { name: "pearl",          ambient: [0.25,      0.20725,   0.20725,   1.0], diffuse: [1.0,        0.829,      0.829,      1.0], specular: [0.296648, 0.296648, 0.296648, 1.0], shininess: 0.088      * 128.0 },
    { name: "copper",         ambient: [0.19125,   0.0735,    0.0225,    1.0], diffuse: [0.7038,     0.27048,    0.0828,     1.0], specular: [0.256777, 0.137622, 0.086014, 1.0], shininess: 0.1        * 128.0 },
    { name: "red plastic",    ambient: [0.0,       0.0,       0.0,       1.0], diffuse: [0.5,        0.0,        0.0,        1.0], specular: [0.7,      0.6,      0.6,      1.0], shininess: 0.25       * 128.0 },
    { name: "red rubber",     ambient: [0.05,      0.0,       0.0,       1.0], diffuse: [0.5,        0.4,        0.4,        1.0], specular: [0.7,      0.04,     0.04,     1.0], shininess: 0.078125   * 128.0 },
    // row 5
    { name: "ruby",           ambient: [0.1745,    0.01175,   0.01175,   1.0], diffuse: [0.61424,    0.04136,    0.04136,    1.0], specular: [0.727811, 0.626959, 0.626959, 1.0], shininess: 0.6        * 128.0 },
    { name: "gold",           ambient: [0.24725,   0.1995,    0.0745,    1.0], diffuse: [0.75164,    0.60648,    0.22648,    1.0], specular: [0.628281, 0.555802, 0.366065, 1.0], shininess: 0.4        * 128.0 },
    { name: "white plastic",  ambient: [0.0,       0.0,       0.0,       1.0], diffuse: [0.55,       0.55,       0.55,       1.0], specular: [0.7,      0.7,      0.7,      1.0], shininess: 0.25       * 128.0 },
    { name: "white rubber",   ambient: [0.05,      0.05,      0.05,      1.0], diffuse: [0.5,        0.5,        0.5,        1.0], specular: [0.7,      0.7,      0.7,      1.0], shininess: 0.078125   * 128.0 },
    // row 6
    { name: "turquoise",      ambient: [0.1,       0.18725,   0.1745,    1.0], diffuse: [0.396,      0.74151,    0.69102,    1.0], specular: [0.297254, 0.30829,  0.306678, 1.0], shininess: 0.1        * 128.0 },
    { name: "silver",         ambient: [0.19225,   0.19225,   0.19225,   1.0], diffuse: [0.50754,    0.50754,    0.50754,    1.0], specular: [0.508273, 0.508273, 0.508273, 1.0], shininess: 0.4        * 128.0 },
    { name: "yellow plastic", ambient: [0.0,       0.0,       0.0,       1.0], diffuse: [0.5,        0.5,        0.0,        1.0], specular: [0.6,      0.6,      0.5,      1.0], shininess: 0.25       * 128.0 },
    { name: "yellow rubber",  ambient: [0.05,      0.05,      0.0,       1.0], diffuse: [0.5,        0.5,        0.4,        1.0], specular: [0.7,      0.7,      0.04,     1.0], shininess: 0.078125   * 128.0 }
];

const sphere_x = [50, 250, 450, 650]; // 4 columns
const sphere_y = [30, 126, 222, 318, 414, 510]; // 6 rows

// Material related macros
const material_size = 64;       // This is size of one material struct containing 4 vec4's
const material_stride = 256;    // Minimum byte offset alignment hardware limit is 256, we are using it as padding after 64 bytes above struct for each material, so 64+192 =256 per spehere
const num_spheres = 24; 

// 64 + 64 + 64 (mvp) + 
// 16 + 16 + 16 + 16 + 16 + 16 + 16 + 16 (Light + Amibient + Diffuse + Specular + Position + Material Ambient + Material Diffuse + Material Specular + Material Shininess + LKeyPressed) = 320 bytes
// 16 lightKeyPressed = 336 bytes

let perspectiveProjectionMatrix = null;
let depthTexture = null;

let sphere = null;
let numIndices = 0;
let buffer_position = null;
let buffer_normals = null;
let buffer_texcoords = null;
let buffer_elements = null;

// Animation related variables
// To start animation -> To requestAnimationFrame() to be called cross-browser compatible
var requestAnimationFrame = window.requestAnimationFrame || window.webkitRequestAnimationFrame ||
    window.mozRequestAnimationFrame || 
    window.oRequestAnimationFrame || 
    window.msRequestAnimationFrame;


// To stop animation -> To call cancelAnimation frame
var cancelAnimationFrame = window.cancelAnimationFrame || 
                        window.webkitCancelRequestAnimationFrame || window.webkitCancelAnimationFrame || 
                        window.mozCancelRequestAnimationFrame || window.mozCancelAnimationFrame || 
                        window.oCancelRequestAnimationFrame || window.oCancelAnimationFrame || 
                        window.msCancelRequestAnimationFrame ||window.msCancelAnimationFrame;

// onLoad() function
// To avoid browser to get locked by long waiting for GPU to be completed, 
// we will use async function and await keyword to wait for GPU to be ready
async function main() {
    // Get canvas element
    canvas = document.getElementById("AMC");
    if(!canvas)
        console.log("Obtaining canvas failed");
    else
        console.log("Obtained canvas successfully");

    // Store original canvas width and height
    canvas_original_width = canvas.width;
    canvas_original_height = canvas.height;

    // Register event handlers
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseDown, false);
    window.addEventListener("resize", resize, false);

    // Best practices for WebGPU during fullscreen
    document.addEventListener("fullscreenchange", onFullScreenChange, false);
    document.addEventListener("webkitfullscreenchange", onFullScreenChange, false);

    // Initialize WebGPU
    // Step 1 Get GPU interface from navigator
    const tsGPU = navigator.gpu;
    if(null == tsGPU)
    {
        console.log("WebGPU is not supported on this browser\n");
        throw new Error("WebGPU is not supported on this browser\n");
        return;
    }
    else{
        console.log("WebGPU is supported on this browser\n");
    }

    // step 2 Get GPU adapter from GPU interface
    const tsAdapter = await tsGPU.requestAdapter();
    if(null == tsAdapter)
    {
        console.log("Obtaining GPU adapter failed\n");
        throw new Error("Obtaining GPU adapter failed\n");
        return;
    }

    // step 3 Get GPU device from GPU adapter
    device = await tsAdapter.requestDevice();
    if(null == device)
    {
        console.log("Obtaining GPU device failed\n");
        throw new Error("Obtaining GPU device failed\n");
        return;
    }

    // step 4 Get GPU queue from GPU device
    // As browsers can be work on different devices may get lost such as due to reset, switch off, etc. 
    // So, we will check if the device is lost or not
    // In such case, we may not have capturable errormessage so register one generic error handler 
    // to get the error message
    device.addEventListener("uncapturederror", onUncapturedError);
    
    // Register a specific device lost handler to get the error message when device is lost
    device.lost.then(onDeviceLost);

    // Call stub function from here
    initialize();
    resize();
    draw();
}

// Event handlers
function onUncapturedError(event) {
    console.error("WebGPU onUncapturedError(): " + event.error.message);
}

function onDeviceLost(info) {
    console.warn("WebGPU onDeviceLost() device lost reason: " + info.reason + " message: " + info.message);
    device = null;
    context = null;
    queue = null;
    canvas_format = null;
    animation_frame_id = null;

    sphere = null;
    numIndices = 0;
    buffer_position = null;
    buffer_normals = null;
    buffer_texcoords = null;
    buffer_elements = null;

    render_pipeline = null;
    buffer_uniform = null;
    bindingGroups_uniform = null;
    buffer_material = null;
    bindingGroups_material = null;
    perspectiveProjectionMatrix = null;
    depthTexture = null;
}

function toggleFullScreen() {
    var fullscreen_element = document.fullscreenElement ||
        document.webkitFullscreenElement ||
        document.mozFullScreenElement ||
        document.msFullscreenElement ||
        null;

    if(fullscreen_element == null){

        if(canvas.requestFullscreen)
            canvas.requestFullscreen();
        else if(canvas.mozRequestFullScreen)
            canvas.mozRequestFullScreen();
        else if(canvas.webkitRequestFullScreen)
            canvas.webkitRequestFullScreen();
        else if(canvas.msRequestFullscreen)
            canvas.msRequestFullscreen();

       // In web GL we initialize bFullScreen here not thinking about async operations
       // But in WebGPU, considering cross browser full screen compatibility we will do this on fullScreenchange event handler

    } else {
        if(document.exitFullscreen)
            document.exitFullscreen()
        else if(document.mozCancelFullScreen)
            document.mozCancelFullScreen();
        else if (document.webkitExitFullscreen)
            document.webkitExitFullscreen();
        else if(document.msExitFullscreen)
            document.msExitFullscreen();

        // In web GL we initialize bFullScreen here not thinking about async operations
       // But in WebGPU, considering cross browser full screen compatibility we will do this on fullScreenchange event handler
    }
}

function onFullScreenChange() {

    // Code
    var fullscreen_element = document.fullscreenElement ||
        document.webkitFullscreenElement ||
        document.mozFullScreenElement ||
        document.msFullscreenElement ||
        null;

    if(fullscreen_element == null){
        bFullScreen = false;
    }   
    else{
        bFullScreen = true;
    }

    // Call resize() here because when we go to full screen or come out of full screen, we need to resize the canvas and re-render the scene
    resize();
}

// After recieving the gpu adapter & device, 
// Remember getting queue is always synchronous never fail if we already successfully have GPU, adapter and device
// So there is no need of await and error checking
function initialize() {
    // Code
    // step 5 Get GPU queue from GPU device
    queue = device.queue;
    console.log("Obtaining GPU queue successfully\n");

    // step 6 Get WebGPU context from canvas
    context = canvas.getContext("webgpu");
    if(context == null)
    {
        console.log("Obtaining WebGPU context failed\n");
        throw new Error("Obtaining WebGPU context failed\n");
        return;
    }

    // step 7 Get WebGPU preferred canvas format from context
    // gpu texture format is a string that represents the format of the texture, such as "rgba8unorm" or "bgra8unorm"
    canvas_format = navigator.gpu.getPreferredCanvasFormat();
    console.log("Obtaining WebGPU preferred canvas format successfully\n");

    // step 8 Configure the canvas with the device and format to suit our needs
    const canvas_configuration = {
        device: device,
        format: canvas_format,
        usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_SRC,
        alphaMode: "opaque"
    };  
    context.configure(canvas_configuration);
    console.log("initialize() Configuring WebGPU context successfully done with format: canvas_format\n");

    // Vertex shader code source in WGSL (WebGPU Shading Language)
    const vertexShaderSourceCode = "struct MyUniformData\n" +
    "{\n" +
        "modelMatrix : mat4x4<f32>,\n" +
        "viewMatrix : mat4x4<f32>,\n" +
        "projectionMatrix : mat4x4<f32>,\n" +
        "lightAmbient : vec4<f32>,\n" +
        "lightDiffuse : vec4<f32>,\n" +
        "lightSpecular : vec4<f32>,\n" +
        "lightPosition : vec4<f32>,\n" +
        "lKeyPressed : vec4<u32>,\n" +
    "};\n" +
    "struct VertexOutput\n" +
    "{\n" +
        "@builtin(position) Position : vec4<f32>,\n" +
        "@location(0) transformedNormal : vec3<f32>,\n" +
        "@location(1) lightDirection : vec3<f32>,\n" +
        "@location(2) viewerVector : vec3<f32>\n" +
    "};\n" +    
    "@group(0) @binding(0) var<uniform> uMyUniformData : MyUniformData;\n" +
    "@vertex\n" +
    "fn main(@location(0) pos : vec3<f32>, @location(1) normal : vec3<f32>) -> VertexOutput\n"+
    "{\n"+
        "var output : VertexOutput;\n"+
        "let eyeCoordinates : vec4<f32> = uMyUniformData.viewMatrix * uMyUniformData.modelMatrix * vec4<f32>(pos, 1.0);\n" +
        "let modelViewMatrix : mat3x3<f32> = mat3FromMat4(uMyUniformData.viewMatrix * uMyUniformData.modelMatrix);\n" +
        "let normalMatrix : mat3x3<f32> = transpose(inverse3x3(modelViewMatrix));\n" +
        "output.transformedNormal  = normalize(normalMatrix * normal);\n" +
        "output.lightDirection  = normalize(uMyUniformData.lightPosition.xyz - eyeCoordinates.xyz);\n" +
        "output.viewerVector = normalize(-eyeCoordinates.xyz);\n" +
        "let reflectionVector : vec3<f32> = reflect(-output.lightDirection, output.transformedNormal);\n" +
        "output.Position = uMyUniformData.projectionMatrix * uMyUniformData.viewMatrix * uMyUniformData.modelMatrix * vec4<f32>(pos, 1.0);\n" +
      "return output;\n" +
    "}\n" +
    "fn mat3FromMat4(m:mat4x4<f32>)->mat3x3<f32>\n"+
    "{\n"+
        "return(mat3x3<f32>(m[0].xyz, m[1].xyz, m[2].xyz));\n"+
    "}\n"+
    "fn inverse3x3(m:mat3x3<f32>)->mat3x3<f32>\n"+
    "{\n"+
    "let determinant = m[0][0] * (m[1][1]*m[2][2] - m[2][1]*m[1][2]) - "+
    "                  m[1][0] * (m[0][1]*m[2][2] - m[2][1]*m[0][2]) + "+
    "                  m[2][0] * (m[0][1]*m[1][2] - m[1][1]*m[0][2]);\n"+
    "if(determinant == 0.0)\n"+
    "{\n"+
    "    return(mat3x3<f32>(vec3<f32>(0.0), vec3<f32>(0.0), vec3<f32>(0.0)));\n"+
    "}\n"+
    "let inverse_determinant = 1.0 / determinant;\n"+
    "let column0 = vec3<f32>\n"+
    "(\n"+
    "    (m[1][1]*m[2][2] - m[2][1]*m[1][2]) * inverse_determinant,\n"+
    "    (m[2][1]*m[0][2] - m[0][1]*m[2][2]) * inverse_determinant,\n"+
    "    (m[0][1]*m[1][2] - m[1][1]*m[0][2]) * inverse_determinant"+
    ");\n"+
    "let column1 = vec3<f32>\n"+
    "(\n"+
    "    (m[2][0]*m[1][2] - m[1][0]*m[2][2]) * inverse_determinant,\n"+
    "    (m[0][0]*m[2][2] - m[2][0]*m[0][2]) * inverse_determinant,\n"+
    "    (m[1][0]*m[0][2] - m[0][0]*m[1][2]) * inverse_determinant"+
    ");\n"+
    "let column2 = vec3<f32>\n"+
    "(\n"+
    "    (m[1][0]*m[2][1] - m[2][0]*m[1][1]) * inverse_determinant,\n"+
    "    (m[2][0]*m[0][1] - m[0][0]*m[2][1]) * inverse_determinant,\n"+
    "    (m[0][0]*m[1][1] - m[1][0]*m[0][1]) * inverse_determinant"+
    ");\n"+
    "return(mat3x3<f32>(column0, column1, column2));\n"+
    "}\n";

    // Create GPUShaderModuleDescriptor type for the vertex shader
    const shaderModuleDescriptor_vertex = {
        code: vertexShaderSourceCode
    };

    // Create GPUShaderModule type for the vertex shader
    const shaderModule_vertex = device.createShaderModule(shaderModuleDescriptor_vertex);
    if(shaderModule_vertex == null)
    {
        console.log("initialize() Creating GPUShaderModule for vertex shader failed\n");
        throw new Error("initialize() Creating GPUShaderModule for vertex shader failed");
        return;
    }
    else
    {
        console.log("initialize() Creating GPUShaderModule for vertex shader successful\n");
    }

    // Fragment shader code source in WGSL (WebGPU Shading Language)
    const fragmentShaderSourceCode = "struct MyUniformData\n" +
    "{\n" +
        "modelMatrix : mat4x4<f32>,\n" +
        "viewMatrix : mat4x4<f32>,\n" +
        "projectionMatrix : mat4x4<f32>,\n" +
        "lightAmbient : vec4<f32>,\n" +
        "lightDiffuse : vec4<f32>,\n" +
        "lightSpecular : vec4<f32>,\n" +
        "lightPosition : vec4<f32>,\n" +
        "lKeyPressed : vec4<u32>,\n" +
    "};\n" +
    "struct Material\n" +
    "{\n" +        
        "materialAmbient : vec4<f32>,\n" +
        "materialDiffuse : vec4<f32>,\n" +
        "materialSpecular : vec4<f32>,\n" +
        "materialShininess : vec4<f32>,\n" +
    "};\n" +
    "struct VertexOutput\n" +
    "{\n" +
        "@builtin(position) Position : vec4<f32>,\n" +
        "@location(0) transformedNormal : vec3<f32>,\n" +
        "@location(1) lightDirection : vec3<f32>,\n" +
        "@location(2) viewerVector : vec3<f32>\n" +
    "};\n" +    
    "@group(0) @binding(0) var<uniform> uMyUniformData : MyUniformData;\n" +
    "@group(1) @binding(0) var<uniform> material : Material;\n" +
    "@fragment\n" +
    "fn main(output: VertexOutput) -> @location(0) vec4<f32>\n" +
    "{\n" +
        "var phong_ads_color : vec3<f32>;\n" +
        "if(uMyUniformData.lKeyPressed.x == 1u)\n" +   // WGSL is strictly "typed" with no implicit type conversion or type promotion, so we need to use "u" suffix for unsigned integer literal
        "{\n" +
            "let normalized_transformedNormal : vec3<f32> = normalize(output.transformedNormal);\n" +
            "let normalized_lightDirection : vec3<f32> = normalize(output.lightDirection);\n" +
            "let normalized_viewerVector : vec3<f32> = normalize(output.viewerVector);\n" +
            "let ambient : vec3<f32> = uMyUniformData.lightAmbient.xyz * material.materialAmbient.xyz;\n" +
            "let diffuse : vec3<f32> = uMyUniformData.lightDiffuse.xyz * material.materialDiffuse.xyz * max(dot(normalized_lightDirection, normalized_transformedNormal), 0.0);\n" +
            "let reflectionVector: vec3<f32> = -normalized_lightDirection * normalized_transformedNormal * 2.0 * dot(normalized_lightDirection, normalized_transformedNormal) + normalized_lightDirection;\n" +
            "let normalized_reflectionVector : vec3<f32> = normalize(reflectionVector);\n" +
            "let specular : vec3<f32> = uMyUniformData.lightSpecular.xyz * material.materialSpecular.xyz * pow(max(dot(normalized_reflectionVector, normalized_viewerVector), 0.0), material.materialShininess.x);\n" +
            "phong_ads_color = ambient + diffuse + specular;\n" +
        "}\n" +
        "else" +
        "{\n" +
            "phong_ads_color = vec3<f32>(1.0, 1.0, 1.0);\n" +
            "}\n" +
        "return vec4<f32>(phong_ads_color, 1.0);\n" + // color
    "}\n";

    // Create GPUShaderModuleDescriptor type for the fragment shader
    const shaderModuleDescriptor_fragment = {
        code: fragmentShaderSourceCode
    };

    // Create GPUShaderModule type for the fragment shader
    const shaderModule_fragment = device.createShaderModule(shaderModuleDescriptor_fragment);
    if(shaderModule_fragment == null)
    {
        console.log("initialize() Creating GPUShaderModule for fragment shader failed\n");
        throw new Error("initialize() Creating GPUShaderModule for fragment shader failed");
        return;
    }
    else
    {
        console.log("initialize() Creating GPUShaderModule for fragment shader successful\n");
    }

    // Create a sphere mesh using the Mesh class
    sphere = new Mesh();

    // Make sphere mesh with radius 1.0, 30 latitude segments, and 30 longitude segments
    makeSphere(sphere, 2.0, 30, 30);
    numIndices = sphere.getIndexCount();
    console.log("initialize() Sphere geometry = vertexCount = " + sphere.getVertexCount() + ", indexCount = " + numIndices + " mesh created successfully\n");

    const meshData = sphere.getMeshData();

    // Create the GPU buffer for vertex positions GPUBuffer type
    buffer_position = createVertexBuffer(meshData.verticesArray);

    if(buffer_position == null)
    {
        console.log("initialize() Creating GPU buffer for vertex positions failed\n");
        throw new Error("initialize() Creating GPU buffer for vertex positions failed\n");
        return;
    }

    buffer_normals = createVertexBuffer(meshData.normalsArray);
    if(buffer_normals == null)
    {
        console.log("initialize() Creating GPU buffer for vertex normals failed\n");
        throw new Error("initialize() Creating GPU buffer for vertex normals failed\n");
        return;
    }

    buffer_texcoords = createVertexBuffer(meshData.texCoordsArray);
    if(buffer_texcoords == null)
    {
        console.log("initialize() Creating GPU buffer for vertex texture coordinates failed\n");
        throw new Error("initialize() Creating GPU buffer for vertex texture coordinates failed\n");
        return;
    }

    buffer_elements = createIndexBuffer(meshData.indicesArray);
    if(buffer_elements == null)
    {
        console.log("initialize() Creating GPU buffer for vertex elements failed\n");
        throw new Error("initialize() Creating GPU buffer for vertex elements failed\n");
        return;
    }

    // Bind group layout enter GPUBindGroupLayoutEntry type is common for both triangle and square, so we can use same bind group layout for both
    const bindGroupLayout_myUniform = createBindGroupLayoutForUniform(0, GPUShaderStage.VERTEX | GPUShaderStage.FRAGMENT, "uniform");

    if(bindGroupLayout_myUniform == null)
    {
        console.log("initialize() Creating GPUBindGroupLayout for my uniform failed\n");
        throw new Error("initialize() Creating GPUBindGroupLayout for my uniform failed\n");
        return;
    }   

    const myUniformBufferSize = Float32Array.BYTES_PER_ELEMENT * 16 +   // Model Matrix: from 0th to 63rd byte offset
                                Float32Array.BYTES_PER_ELEMENT * 16 +   // View Matrix: from 64th to 127th byte offset
                                Float32Array.BYTES_PER_ELEMENT * 16 +   // Projection Matrix: from 128th to 191st byte offset

                                Float32Array.BYTES_PER_ELEMENT * 4 +    // Light Ambient: from 192nd to 207th byte offset
                                Float32Array.BYTES_PER_ELEMENT * 4 +    // Light Diffuse: from 208th to 223rd byte offset
                                Float32Array.BYTES_PER_ELEMENT * 4 +    // Light Specular: from 224th to 239th byte offset
                                Float32Array.BYTES_PER_ELEMENT * 4 +    // Light Position: from 240th to 255th byte offset

                                Uint32Array.BYTES_PER_ELEMENT * 4;      // LKeyPressed: from 256th to 271th byte offset

    buffer_uniform = createUniformBuffer(myUniformBufferSize, GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST);

    // Create GPUBufferBinding type for the my uniform buffer
    // create bind group for my uniform for triangle
    bindingGroups_uniform = createBindGroupForUniform(buffer_uniform, 0, myUniformBufferSize, 0, bindGroupLayout_myUniform);

    const materialBufferSize = num_spheres * material_stride;   // 24 * 256 = 6144 (where 256 is 64 material + 192 padding)

    buffer_material = createUniformBuffer(materialBufferSize, GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST);

    // Now we will write the material table into buffer_material. But to do that we need to convert whole name:value style material table into a float array
    const stride = material_stride / Float32Array.BYTES_PER_ELEMENT;    // 256 / 4 = 64
    const materialArray = new Float32Array(num_spheres * stride);  // 24 * 64 = 1536

    for(var i = 0; i <num_spheres; i++)
    {
        materialArray.set(materialTable[i].ambient, i * stride + 0);
        materialArray.set(materialTable[i].diffuse, i * stride + 4);
        materialArray.set(materialTable[i].specular, i * stride + 8);
        materialArray.set([materialTable[i].shininess, 0.0, 0.0, 0.0], i * stride + 12);
    }

    queue.writeBuffer(buffer_material, 0, materialArray, 0, materialArray.length);

    // Now create bind group layout entry for material bind group
    const bindgroupLayoutEntry_material = 
    {
        binding : 0,
        visibility: GPUShaderStage.FRAGMENT,
        buffer: 
        {
            type: "uniform",
            hasDynamicOffset: true,
            minBindingSize: material_size   // 64 one material struct size
        }
    };

    // Now from above entry create bind group layout descriptor for material
    const bindGroupLayoutDescriptor_material = 
    {
        entries: [bindgroupLayoutEntry_material]
    };

    const bindgroupLayout_material = device.createBindGroupLayout(bindGroupLayoutDescriptor_material);
    if(null == bindgroupLayout_material)
    {
        console.log("[ERROR] failed createBindGroupLayout() for bindGroupLayoutDescriptor_material");
    }

    // create bindgroup entry for material
    const bindgroupEntry_material = 
    {
        binding: 0,
        resource: 
        {
            buffer: buffer_material,
            offset: 0,
            size: material_size // 64
        }
    };

    // Create bind group descriptor for material
    const bindgroupDescriptor_material = 
    {
        layout: bindgroupLayout_material,
        entries: [bindgroupEntry_material],
    };

    // create actual bind group material
    bindingGroups_material = device.createBindGroup(bindgroupDescriptor_material);
    if(null == bindingGroups_material)
    {
        console.log("[ERROR] failed createBindGroup() for bindgroupDescriptor_material\n");
    }

    // Create GPUPipelineLayoutDescriptor type
    const pipelineLayoutDescriptor = {
        bindGroupLayouts: [bindGroupLayout_myUniform, bindgroupLayout_material]
    };

    // Create GPUPipelineLayout type
    const pipelineLayout = device.createPipelineLayout(pipelineLayoutDescriptor);

    if(pipelineLayout == null)
    {
        console.log("initialize() Creating GPUPipelineLayout failed\n");
        throw new Error("initialize() Creating GPUPipelineLayout failed\n");
        return;
    }   

    // create GPUVertexBufferAttribute type for the vertex position buffer
    const positionVertexBufferAttribute = {
        shaderLocation: 0, // This matches with the 0th @location(0) in the vertex shader code
        offset: 0,
        format: "float32x3" // vec3<f32> is represented as float32x3 in WebGPU for R32G32B32 format
    };

    // create GPUVertexBufferLayout type for the vertex position buffer
    const positionVertexBufferLayout = {
        arrayStride: Float32Array.BYTES_PER_ELEMENT * 3, // 4 * 3 floats * 4 bytes per float
        attributes: [positionVertexBufferAttribute],
        stepMode: "vertex"  // jump vertex by vertex, not instance by instance
    };

    const normalVertexBufferAttribute = {
        shaderLocation: 1, // This matches with the 1st @location(1) in the vertex shader code
        offset: 0,
        format: "float32x3" // vec3<f32> is represented as float32x3 in WebGPU for R32G32B32 format
    };

    // Create GPUVertexBufferLayout type for the vertex normal buffer
    const normalVertexBufferLayout = {
        arrayStride: Float32Array.BYTES_PER_ELEMENT * 3, // 4 * 3 floats * 4 bytes per float
        attributes: [normalVertexBufferAttribute],
        stepMode: "vertex"  // jump vertex by vertex, not instance by instance
    };

    // Create GPUVertexState type for the vertex shader stage
    const vertexShaderState = {
        module: shaderModule_vertex,
        entryPoint: "main",
        buffers: [positionVertexBufferLayout, normalVertexBufferLayout]
    };

    // Create GPUColorTargetState type for the fragment shader output
    const colorTargetState = {
        format: canvas_format
    };

    // Create GPUFragmentState type for the fragment shader stage
    const fragmentShaderState = {
        module: shaderModule_fragment,
        entryPoint: "main",
        targets: [colorTargetState]
    };

    // Create Primitive state for the render pipeline
    const primitiveState = {
        topology: "triangle-list",
        stripIndexFormat: undefined,
        frontFace: "ccw",   // ccw = counter-clockwise winding order for front face, cw = clockwise winding order for front face
        cullMode: "none"    // none = no culling, front = cull front face, back = cull back face
    };

    // Depth and Stencil State
    const depthStencilState = {
        depthWriteEnabled: true,
        depthCompare: "less-equal", // glsl: GL_LEQUAL, Vulkan: VK_COMPARE_OP_LESS_OR_EQUAL, DirectX: D3D12_COMPARISON_FUNC_LESS_EQUAL
        format: "depth24plus-stencil8" // glsl: GL_DEPTH24_STENCIL8, Vulkan: VK_FORMAT_D24_UNORM_S8_UINT, DirectX: DXGI_FORMAT_D24_UNORM_S8_UINT
    };

    // create render pipeline descriptor of GPURenderPipelineDescriptor type
    const renderPipelineDescriptor = {
        layout: pipelineLayout,
        vertex: vertexShaderState,
        fragment: fragmentShaderState,
        primitive: primitiveState,
        depthStencil: depthStencilState
    };

    // Create render pipeline of GPURenderPipeline type
    render_pipeline = device.createRenderPipeline(renderPipelineDescriptor);
    if(render_pipeline == null)
    {
        console.log("initialize() Creating GPURenderPipeline failed\n");
        throw new Error("initialize() Creating GPURenderPipeline failed\n");
        return;
    }

    // create perspective projection matrix using gl-matrix library
    perspectiveProjectionMatrix = mat4.create();

    // step 9 Set clear color to dark gray
    clear_color = {r: 0.5, g: 0.5, b: 0.5, a: 1.0};
}

// Create vertex buffer UDF
function createVertexBuffer(_vertexData) {
    // Code
    const bufferDescriptor = {
        size: _vertexData.byteLength,
        usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
    };

    const buffer = device.createBuffer(bufferDescriptor);
    if(buffer == null)
    {
        console.log("createVertexBuffer() Creating GPU buffer failed\n");
        //throw new Error("createVertexBuffer() Creating GPU buffer failed\n");
        return null;
    }

    queue.writeBuffer(buffer, 0, _vertexData, 0, _vertexData.length);
    console.log("createVertexBuffer() Creating GPU buffer successfully done\n");

    return buffer;
}

// Create index buffer UDF
function createIndexBuffer(_indexData) {
    // Code
    const bufferDescriptor = {
        size: _indexData.byteLength,
        usage: GPUBufferUsage.INDEX | GPUBufferUsage.COPY_DST
    };

    const buffer = device.createBuffer(bufferDescriptor);
    if(buffer == null)
    {
        console.log("createIndexBuffer() Creating GPU buffer failed\n");
        //throw new Error("createIndexBuffer() Creating GPU buffer failed\n");
        return null;
    }

    queue.writeBuffer(buffer, 0, _indexData, 0, _indexData.length);
    console.log("createIndexBuffer() Creating GPU buffer successfully done\n");

    return buffer;
}

function resize(){

    // Code
    if(bFullScreen){
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    } else {
        canvas.width = canvas_original_width;
        canvas.height = canvas_original_height;
    }

    // Depth texture creation
    if(device != null)
    {
        if(depthTexture != null)
        {
            depthTexture.destroy();
            depthTexture = null;
        }

        // To create depth texture, we need to create a GPUTextureDescriptor type
        const depthTextureDescriptor = {
            //       width,        height, depthOrArrayLayers
            size: [canvas.width, canvas.height, 1],
            dimension: "2d",
            format: "depth24plus-stencil8",
            usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_SRC
        };

        // Now create the depth texture using device.createTexture() method
        depthTexture = device.createTexture(depthTextureDescriptor);

        if(depthTexture == null)
        {
            console.log("resize() Creating depth texture failed\n");
            throw new Error("resize() Creating depth texture failed\n");
            return;
        }
    }

    mat4.perspective(perspectiveProjectionMatrix, 45 * Math.PI / 180, canvas.width / canvas.height, 0.1, 100.0);

    //gl.viewport(0,0,canvas.width,canvas.height);
}

function draw() {
    // Code
    // Device may be lost, initialization may not be done yet
    if(device == null || context == null || queue == null || canvas_format == null)
        return;

    // Display
    // step 10 Create a command encoder from device 
    // to record commands for the GPU due to async nature & possibility of device lost 
    // it is better to create command encoder inside draw() function
    const command_encoder = device.createCommandEncoder();  // GPUCommandEncoder type
    if(command_encoder == null)
    {
        console.log("Obtaining GPU command encoder failed\n");
        throw new Error("Obtaining GPU command encoder failed\n");
        return;
    }

    // Step 11 create render pass color attachment descriptor to clear the canvas with blue color
    const render_pass_color_attachment = {
        view: context.getCurrentTexture().createView(),
        clearValue: clear_color,
        loadOp: "clear",
        storeOp: "store"
    };

    // Depth and Stencil attachment descriptor
    const render_pass_depth_stencil_attachment = {
        view: depthTexture.createView(),
        depthClearValue: 1.0,   // glsl: GL_DEPTH_BUFFER_BIT, Vulkan: VK_IMAGE_ASPECT_DEPTH_BIT, DirectX: D3D12_CLEAR_FLAG_DEPTH        
        depthLoadOp: "clear",   // glsl: GL_CLEAR, Vulkan: VK_ATTACHMENT_LOAD_OP_CLEAR, DirectX: D3D12_CLEAR_FLAG_DEPTH
        depthStoreOp: "store",

        stencilClearValue: 0,   // glsl: GL_STENCIL_BUFFER_BIT, Vulkan: VK_IMAGE_ASPECT_STENCIL_BIT, DirectX: D3D12_CLEAR_FLAG_STENCIL
        stencilLoadOp: "clear",
        stencilStoreOp: "store"
    };

    // Step 12 create render pass descriptor to begin render pass of GPURenderPassDescriptor type
    const render_pass_descriptor = {
        colorAttachments: [render_pass_color_attachment],
        depthStencilAttachment: render_pass_depth_stencil_attachment
    };

    const modelMatrix = mat4.create();
    const viewMatrix = mat4.create();
    const projectionMatrix = perspectiveProjectionMatrix;
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -6.0]);
    // toggle lighting
    if(isLightingEnabled == true)
    {
        lKeyPressed[0] = 1;
    }
    else
    {
        lKeyPressed[0] = 0;
    }

    // 1: Matrices for Model, View and Projection
    queue.writeBuffer(buffer_uniform, 
        0, // modelMatrix from 0th byte offset to 63rd byte offset
        modelMatrix, 
        0, 
        modelMatrix.length);
    
    queue.writeBuffer(buffer_uniform, 
        Float32Array.BYTES_PER_ELEMENT * 16, // viewMatrix from 64th byte offset to 127th byte offset
        viewMatrix, 
        0, 
        viewMatrix.length);
    
    queue.writeBuffer(buffer_uniform, 
            Float32Array.BYTES_PER_ELEMENT * 16 + Float32Array.BYTES_PER_ELEMENT * 16, // projectionMatrix from 128th byte offset to 191st byte offset
        projectionMatrix, 
        0, 
        projectionMatrix.length);
    
        // Reset light position
        lightPosition[0] = 0.0;
        lightPosition[1] = 0.0;
        lightPosition[2] = 0.0;
        lightPosition[3] = 1.0;

        if(accessChoosen == "X")
        {
            lightPosition[0] = 0.0;
            lightPosition[1] = 45.0 * Math.sin(deg2rad(lightAngle));
            lightPosition[2] = 45.0 * Math.cos(deg2rad(lightAngle));;
            lightPosition[3] = 1.0;
        }
        else if(accessChoosen == "Y")
        {
            lightPosition[0] = 45.0 * Math.sin(deg2rad(lightAngle));    // Here there should be cos()
            lightPosition[1] = 0.0;
            lightPosition[2] = 45.0 * Math.cos(deg2rad(lightAngle));;
            lightPosition[3] = 1.0;
        }
        else if(accessChoosen == "Z")
        {
            lightPosition[0] = 45.0 * Math.sin(deg2rad(lightAngle));    // Here there should be cos()
            lightPosition[1] = 45.0 * Math.cos(deg2rad(lightAngle));
            lightPosition[2] = 0.0;
            lightPosition[3] = 1.0;
        }

        if(accessChoosen == "X" || accessChoosen == "Y" || accessChoosen == "Z")
        {
            lightAngle = lightAngle + 1.0;
            if(lightAngle >= 360.0)
            {
                lightAngle = lightAngle - 360.0;
            }
        }

    // 2: Light Ambient, Diffuse, Specular and Position
    queue.writeBuffer(buffer_uniform, 
        Float32Array.BYTES_PER_ELEMENT * 16  * 3, // lightAmbient from 192nd byte offset to 207th byte offset
        lightAmbient, 
        0, 
        lightAmbient.length);

    queue.writeBuffer(buffer_uniform,
        Float32Array.BYTES_PER_ELEMENT * 16  * 3 + Float32Array.BYTES_PER_ELEMENT * 4 * 1, // lightDiffuse from 208th byte offset to 223rd byte offset
        lightDiffuse, 
        0, 
        lightDiffuse.length);
    
    queue.writeBuffer(buffer_uniform,
        Float32Array.BYTES_PER_ELEMENT * 16  * 3 + Float32Array.BYTES_PER_ELEMENT * 4 * 2, // lightSpecular from 224th byte offset to 239th byte offset
        lightSpecular, 
        0, 
        lightSpecular.length);

    queue.writeBuffer(buffer_uniform,
        Float32Array.BYTES_PER_ELEMENT * 16  * 3 + Float32Array.BYTES_PER_ELEMENT * 4 * 3, // lightPosition from 240th byte offset to 255th byte offset
        lightPosition, 
        0, 
        lightPosition.length);    
    
    // 4: LKeyPressed
    queue.writeBuffer(buffer_uniform,
        Float32Array.BYTES_PER_ELEMENT * 16  * 3 + Float32Array.BYTES_PER_ELEMENT * 4 * 4, // lKeyPressed from 256th byte offset to 271th byte offset
        lKeyPressed, 
        0, 
        lKeyPressed.length);

    // Step 13 create render pass encoder to record commands for the GPU
    const render_pass_encoder = command_encoder.beginRenderPass(render_pass_descriptor);

    render_pass_encoder.setPipeline(render_pipeline);
    render_pass_encoder.setVertexBuffer(0, buffer_position);
    render_pass_encoder.setVertexBuffer(1, buffer_normals);
    render_pass_encoder.setIndexBuffer(buffer_elements, "uint16"); // uint16 = 2 bytes per index, uint32 = 4 bytes per index
    
    render_pass_encoder.setScissorRect(0, 0, canvas.width, canvas.height);
    render_pass_encoder.setBindGroup(0, bindingGroups_uniform);

    const viewportWidth = canvas.width / 8.0;
    const viewportHeight = canvas.height / 8.0;

    for(let row = 0; row < 6; row++)
    {
        for(let col = 0; col < 4; col++)
        {
            const X = sphere_x[col];
            const Y = sphere_y[row];
            const viewportX = X * canvas.width / 800;
            const viewportY = Y * canvas.height / 600;

            const i = row * 4 + col;

            render_pass_encoder.setViewport(viewportX, viewportY, viewportWidth, viewportHeight, 0, 1);   // Viewport and scissor rect are same in WebGPU, so we can set viewport only do not have any GPU structure
            //                                                              
            render_pass_encoder.setBindGroup(1, bindingGroups_material, [i * material_stride])
            render_pass_encoder.drawIndexed(numIndices);
        }
    }

    
    //render_pass_encoder.drawIndexed(numIndices); // 3 vertices, 1 instance, first vertex = 0, first instance = 0

    // Step 14 end render pass
    render_pass_encoder.end();    

    // Step 15 finish command encoder to get GPUCommandBuffer type
    // there can be multiple commaand encoder/s and submit to the queue in one go, 
    // but here we have only one command encoder
    queue.submit([command_encoder.finish()]);

    // Step 16 request to call draw() function again for next frame
    animation_frame_id = requestAnimationFrame(draw); // Analogus to image id in Vulkan, DirectX, OpenGL, WebGL

    update(); // Call update() function to update animation related variables for next frame
}

function update() {
    // Code
}

function keyDown(event) {
    // Code
    switch(event.key){      // event.keyCode is deprecated, so using event.key instead and key is a string, so using string values instead of integer key codes
        case "Escape":
            uninitialize();
            window.close(); // may not work in all browsers, so use uninitialize() to clean up resources
            break;
        case "F":
        case "f":
            toggleFullScreen();
            break;
        case "L":
        case "l":
            if(isLightingEnabled == false)
                isLightingEnabled = true;
            else
                isLightingEnabled = false;
            break; 
        case "X":
        case "x":
            accessChoosen = "X";
            break;
        case "Y":
        case "y":
            accessChoosen = "Y";
            break;
        case "Z":
        case "z":
            accessChoosen = "Z";
            break;
        default:
            break; 
    }
}

function mouseDown() {
    // Code
}

function uninitialize() {
    // Code
    // Step 17 Use animation_frame_id for safe cancellation of the animation frame request if it is not null
    if(null != animation_frame_id)
    {
        cancelAnimationFrame(animation_frame_id);
        animation_frame_id = null;
    }

    // Destroy the depth texture
    if(null != depthTexture)
    {
        depthTexture.destroy();
        depthTexture = null;
    }

    // Step 18 Unconfigure & Destroy the context
    if(null != context)
    {
        context.unconfigure();
        context = null;
    }

    // Step 19 Destroy the device
    if(null != device)
    {
        device = null;
        queue = null;
        canvas_format = null;

        buffer_position = null;
        buffer_normals = null;
        buffer_texcoords = null;
        buffer_elements = null;

        render_pipeline = null;
        buffer_uniform = null;
        bindingGroups_uniform = null;
        
        buffer_material = null;
        bindingGroups_material = null;
    }

    sphere = null;
    perspectiveProjectionMatrix = null;

    console.log("WebGPU uninitialization done successfully\n");
}   

// User defined functions
function createBindGroupLayoutForUniform(_bindingIndex, _shaderStageVisibility, _uniformType) {

    // Code

    const bindGroupLayoutEntry = {

        binding: _bindingIndex,

        visibility: _shaderStageVisibility,

        buffer: {

            type: _uniformType

        }

    };

 

    const bindgroupLayoutDescriptor = {

        entries: [bindGroupLayoutEntry]

    };

 

    const bindGroupLayout = device.createBindGroupLayout(bindgroupLayoutDescriptor);

    if(bindGroupLayout == null)

    {

        console.log("createBindGroupLayoutForUniform() Creating bind group layout failed\n");

        throw new Error("createBindGroupLayoutForUniform() Creating bind group layout failed\n");

        return null;

    }

   

    return bindGroupLayout;

}

 

function createUniformBuffer(_uniformBufferSize, _uniformBufferUsage)

{

    // Code

    const bufferDescriptor = {

        size: _uniformBufferSize,

        usage: _uniformBufferUsage

    };

 

    const uniformBuffer = device.createBuffer(bufferDescriptor);

    if(uniformBuffer == null)

    {

        console.log("createUniformBuffer() Creating uniform buffer failed\n");

        throw new Error("createUniformBuffer() Creating uniform buffer failed\n");

        return null;

    }

 

    return uniformBuffer;

}

 

function createBindGroupForUniform(_uniformBuffer, _uniformBufferOffset, _uniformBufferSize, _bindingIndex, _bindGroupLayout)

{

    // Code

    const bufferBinding = {

        buffer: _uniformBuffer,

        offset: _uniformBufferOffset,

        size: _uniformBufferSize

    };

 

    const bindGroupEntry = {

        binding: _bindingIndex,

        resource: bufferBinding

    };

 

    const bindGroupDescriptor = {

        layout: _bindGroupLayout,

        entries: [bindGroupEntry]

    };

 

    const bindGroup = device.createBindGroup(bindGroupDescriptor);

    if(bindGroup == null)

    {

        console.log("createBindGroupForUniform() Creating bind group failed\n");

        throw new Error("createBindGroupForUniform() Creating bind group failed\n");

        return null;

    }
 
    return bindGroup;
}

function deg2rad(degrees) {
    // Code
    return (degrees * Math.PI / 180.0);
}
