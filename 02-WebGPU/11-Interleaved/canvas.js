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

var lightAmbient = new Float32Array([0.1, 0.1, 0.1, 0.0]);
var lightDiffuse = new Float32Array([1.0, 1.0, 1.0, 0.0]);
var lightSpecular = new Float32Array([1.0, 1.0, 1.0, 0.0]);

var lightPosition = new Float32Array([0.0, 0.0, 2.0, 1.0]);

var materialAmbient = new Float32Array([0.0, 0.0, 0.0, 0.0]);
var materialDiffuse = new Float32Array([0.5, 0.2, 0.7, 0.0]);
var materialSpecular = new Float32Array([0.7, 0.7, 0.7, 0.0]);
var materialShininess = new Float32Array([128.0, 0.0, 0.0, 0.0]);

// 64 + 64 + 64 (mvp) + 
// 16 + 16 + 16 + 16 + 16 + 16 + 16 + 16 (Light + Amibient + Diffuse + Specular + Position + Material Ambient + Material Diffuse + Material Specular + Material Shininess + LKeyPressed) = 320 bytes
// 16 lightKeyPressed = 336 bytes

let perspectiveProjectionMatrix = null;
let depthTexture = null;

// For cube
let buffer_interleaved = null;
let texture_marble = null;
let sampler_marble = null;
let bind_group_texture_and_sampler = null;

var angleCube = 0.0;

/*let sphere = null;
let numIndices = 0;
let buffer_position = null;
let buffer_normals = null;
let buffer_texcoords = null;
let buffer_elements = null;*/

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

    render_pipeline = null;
    buffer_uniform = null;
    bindingGroups_uniform = null;
    perspectiveProjectionMatrix = null;
    depthTexture = null;

    buffer_interleaved = null;
    texture_marble = null;
    sampler_marble = null;
    bind_group_texture_and_sampler = null;
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
async function initialize() {
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
        "materialAmbient : vec4<f32>,\n" +
        "materialDiffuse : vec4<f32>,\n" +
        "materialSpecular : vec4<f32>,\n" +
        "materialShininess : vec4<f32>,\n" +
    "};\n" +
    "struct VertexOutput\n" +
    "{\n" +
        "@builtin(position) Position : vec4<f32>,\n" +
        "@location(0) color : vec3<f32>,\n" +
        "@location(1) texcoord : vec2<f32>,\n" +
        "@location(2) transformedNormal : vec3<f32>,\n" +
        "@location(3) lightDirection : vec3<f32>,\n" +
        "@location(4) viewerVector : vec3<f32>\n" +
    "};\n" +    
    "@group(0) @binding(0) var<uniform> uMyUniformData : MyUniformData;\n" +
    "@vertex\n" +
    "fn main(@location(0) pos : vec3<f32>, @location(1) col: vec3<f32>, @location(2) normal : vec3<f32>, @location(3) texcoord : vec2<f32>) -> VertexOutput\n"+
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
        "output.color = col;\n" +
        "output.texcoord = texcoord;\n" +
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
        "materialAmbient : vec4<f32>,\n" +
        "materialDiffuse : vec4<f32>,\n" +
        "materialSpecular : vec4<f32>,\n" +
        "materialShininess : vec4<f32>,\n" +
    "};\n" +
    "struct VertexOutput\n" +
    "{\n" +
        "@builtin(position) Position : vec4<f32>,\n" +
        "@location(0) color : vec3<f32>,\n" +
        "@location(1) texcoord : vec2<f32>,\n" +
        "@location(2) transformedNormal : vec3<f32>,\n" +
        "@location(3) lightDirection : vec3<f32>,\n" +
        "@location(4) viewerVector : vec3<f32>\n" +
    "};\n" +    
    "@group(0) @binding(0) var<uniform> uMyUniformData : MyUniformData;\n" +
    "@group(1) @binding(0) var myTexture2D : texture_2d<f32>;\n" +
    "@group(1) @binding(1) var mySampler : sampler;\n" +
    "@fragment\n" +
    "fn main(output: VertexOutput) -> @location(0) vec4<f32>\n" +
    "{\n" +
        "var phong_ads_color : vec3<f32>;\n" +

        "let normalized_transformedNormal : vec3<f32> = normalize(output.transformedNormal);\n" +
        "let normalized_lightDirection : vec3<f32> = normalize(output.lightDirection);\n" +
        "let normalized_viewerVector : vec3<f32> = normalize(output.viewerVector);\n" +

        "let ambient : vec3<f32> = uMyUniformData.lightAmbient.xyz * uMyUniformData.materialAmbient.xyz;\n" +
        "let diffuse : vec3<f32> = uMyUniformData.lightDiffuse.xyz * uMyUniformData.materialDiffuse.xyz * max(dot(normalized_lightDirection, normalized_transformedNormal), 0.0);\n" +
        "let reflectionVector: vec3<f32> = -normalized_lightDirection * normalized_transformedNormal * 2.0 * dot(normalized_lightDirection, normalized_transformedNormal) + normalized_lightDirection;\n" +
        "let normalized_reflectionVector : vec3<f32> = normalize(reflectionVector);\n" +
        "let specular : vec3<f32> = uMyUniformData.lightSpecular.xyz * uMyUniformData.materialSpecular.xyz * pow(max(dot(normalized_reflectionVector, normalized_viewerVector), 0.0), uMyUniformData.materialShininess.x);\n" +
        "phong_ads_color = ambient + diffuse + specular;\n" +

        "var tex = textureSample(myTexture2D, mySampler, output.texcoord);\n" + 
        "var color = tex * vec4<f32>(output.color, 1.0) * vec4<f32>(phong_ads_color, 1.0);\n" +
        "return color;\n" + // color
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

    const cube_pcnt = new Float32Array([
        // position            color             normal            texcoord
        // top surface
         1.0,  1.0, -1.0,    0.0, 1.0, 0.0,    0.0,  1.0, 0.0,    0.0, 0.0,
        -1.0,  1.0, -1.0,    0.0, 1.0, 0.0,    0.0,  1.0, 0.0,    1.0, 0.0,
        -1.0,  1.0,  1.0,    0.0, 1.0, 0.0,    0.0,  1.0, 0.0,    1.0, 1.0,
        -1.0,  1.0,  1.0,    0.0, 1.0, 0.0,    0.0,  1.0, 0.0,    1.0, 1.0,
         1.0,  1.0,  1.0,    0.0, 1.0, 0.0,    0.0,  1.0, 0.0,    0.0, 1.0,
         1.0,  1.0, -1.0,    0.0, 1.0, 0.0,    0.0,  1.0, 0.0,    0.0, 0.0,
        // bottom surface
         1.0, -1.0,  1.0,    1.0, 0.5, 0.0,    0.0, -1.0, 0.0,    0.0, 0.0,
        -1.0, -1.0,  1.0,    1.0, 0.5, 0.0,    0.0, -1.0, 0.0,    1.0, 0.0,
        -1.0, -1.0, -1.0,    1.0, 0.5, 0.0,    0.0, -1.0, 0.0,    1.0, 1.0,
        -1.0, -1.0, -1.0,    1.0, 0.5, 0.0,    0.0, -1.0, 0.0,    1.0, 1.0,
         1.0, -1.0, -1.0,    1.0, 0.5, 0.0,    0.0, -1.0, 0.0,    0.0, 1.0,
         1.0, -1.0,  1.0,    1.0, 0.5, 0.0,    0.0, -1.0, 0.0,    0.0, 0.0,
        // front surface
         1.0,  1.0,  1.0,    1.0, 0.0, 0.0,    0.0, 0.0,  1.0,    0.0, 0.0,
        -1.0,  1.0,  1.0,    1.0, 0.0, 0.0,    0.0, 0.0,  1.0,    1.0, 0.0,
        -1.0, -1.0,  1.0,    1.0, 0.0, 0.0,    0.0, 0.0,  1.0,    1.0, 1.0,
        -1.0, -1.0,  1.0,    1.0, 0.0, 0.0,    0.0, 0.0,  1.0,    1.0, 1.0,
         1.0, -1.0,  1.0,    1.0, 0.0, 0.0,    0.0, 0.0,  1.0,    0.0, 1.0,
         1.0,  1.0,  1.0,    1.0, 0.0, 0.0,    0.0, 0.0,  1.0,    0.0, 0.0,
        // back surface
         1.0, -1.0, -1.0,    1.0, 1.0, 0.0,    0.0, 0.0, -1.0,    0.0, 0.0,
        -1.0, -1.0, -1.0,    1.0, 1.0, 0.0,    0.0, 0.0, -1.0,    1.0, 0.0,
        -1.0,  1.0, -1.0,    1.0, 1.0, 0.0,    0.0, 0.0, -1.0,    1.0, 1.0,
        -1.0,  1.0, -1.0,    1.0, 1.0, 0.0,    0.0, 0.0, -1.0,    1.0, 1.0,
         1.0,  1.0, -1.0,    1.0, 1.0, 0.0,    0.0, 0.0, -1.0,    0.0, 1.0,
         1.0, -1.0, -1.0,    1.0, 1.0, 0.0,    0.0, 0.0, -1.0,    0.0, 0.0,
        // left surface
        -1.0,  1.0,  1.0,    0.0, 0.0, 1.0,   -1.0, 0.0, 0.0,    0.0, 0.0,
        -1.0,  1.0, -1.0,    0.0, 0.0, 1.0,   -1.0, 0.0, 0.0,    1.0, 0.0,
        -1.0, -1.0, -1.0,    0.0, 0.0, 1.0,   -1.0, 0.0, 0.0,    1.0, 1.0,
        -1.0, -1.0, -1.0,    0.0, 0.0, 1.0,   -1.0, 0.0, 0.0,    1.0, 1.0,
        -1.0, -1.0,  1.0,    0.0, 0.0, 1.0,   -1.0, 0.0, 0.0,    0.0, 1.0,
        -1.0,  1.0,  1.0,    0.0, 0.0, 1.0,   -1.0, 0.0, 0.0,    0.0, 0.0,
        // right surface
         1.0,  1.0, -1.0,    1.0, 0.0, 1.0,    1.0, 0.0, 0.0,    0.0, 0.0,
         1.0,  1.0,  1.0,    1.0, 0.0, 1.0,    1.0, 0.0, 0.0,    1.0, 0.0,
         1.0, -1.0,  1.0,    1.0, 0.0, 1.0,    1.0, 0.0, 0.0,    1.0, 1.0,
         1.0, -1.0,  1.0,    1.0, 0.0, 1.0,    1.0, 0.0, 0.0,    1.0, 1.0,
         1.0, -1.0, -1.0,    1.0, 0.0, 1.0,    1.0, 0.0, 0.0,    0.0, 1.0,
         1.0,  1.0, -1.0,    1.0, 0.0, 1.0,    1.0, 0.0, 0.0,    0.0, 0.0
    ]);

    // vertex buffer for above interleaved array
    buffer_interleaved = createVertexBuffer(cube_pcnt);
    if(buffer_interleaved == null)
    {
        console.log("initialize() Creating GPU buffer for vertex buffer failed\n");
        throw new Error("initialize() Creating GPU buffer for vertex buffer failed\n");
        return;
    }    
    
    const myUniformBufferSize = Float32Array.BYTES_PER_ELEMENT * 16 * 3 +   // Model Matrix: from 0th to 63rd byte offset // View Matrix: from 64th to 127th byte offset
                                 // Projection Matrix: from 128th to 191st byte offset

                                Float32Array.BYTES_PER_ELEMENT * 4 * 8;    // Light Ambient: from 192nd to 207th byte offset   // Light Diffuse: from 208th to 223rd byte offset
                                 // Light Specular: from 224th to 239th byte offset // Light Position: from 240th to 255th byte offset

                                  // Material Ambient: from 256th to 271st byte offset
                                  // Material Diffuse: from 272nd to 287th byte offset
                                 // Material Specular: from 288th to 303rd byte offset
                                    // Material Shininess: from 304th to 319th byte offset

    buffer_uniform = createUniformBuffer(myUniformBufferSize, GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST);

    // Bind group layout enter GPUBindGroupLayoutEntry type is common for both triangle and square, so we can use same bind group layout for both
    const bindGroupLayout_myUniform = createBindGroupLayoutForUniform(0, GPUShaderStage.VERTEX | GPUShaderStage.FRAGMENT, "uniform");

    if(bindGroupLayout_myUniform == null)
    {
        console.log("initialize() Creating GPUBindGroupLayout for my uniform failed\n");
        throw new Error("initialize() Creating GPUBindGroupLayout for my uniform failed\n");
        return;
    }

    // Create GPUBufferBinding type for the my uniform buffer
    // create bind group for my uniform for triangle
    bindingGroups_uniform = createBindGroupForUniform(buffer_uniform, 0, myUniformBufferSize, 0, bindGroupLayout_myUniform);

    texture_marble = await loadTexture("marble.png");
    if(texture_marble == null)
    {
        console.log("initialize() Loading texture failed\n");
        throw new Error("initialize() Loading texture failed\n");
        return;
    }
    else
    {
        console.log("initialize() Loading texture succeeded\n");
    }

    const samplerDescriptor = {
        magFilter: "linear",
        minFilter: "linear"
    };

    // Create texture sampler
    sampler_marble = device.createSampler(samplerDescriptor);
    if(sampler_marble == null)
    {
        console.log("initialize() Creating texture sampler for square failed\n");
        throw new Error("initialize() Creating texture sampler for square failed\n");
        return;
    }
    else
    {
        console.log("initialize() Creating texture sampler for square succeeded\n");
    }
    const bindGroupLayoutDescriptor_texture_and_sampler = createBindGroupLayoutForTextureAndSampler(
        "float",
        "2d",
        false,
        0,
        GPUShaderStage.FRAGMENT,
        "filtering",
        1,
        GPUShaderStage.FRAGMENT
    );

    // Create texture and sampler bind group
    bind_group_texture_and_sampler = createBindGroupForTextureAndSampler(
        0,
        texture_marble,
        1,
        sampler_marble, 
        bindGroupLayoutDescriptor_texture_and_sampler
    );

    // Create GPUPipelineLayoutDescriptor type
    const pipelineLayoutDescriptor = {
        bindGroupLayouts: [bindGroupLayout_myUniform, bindGroupLayoutDescriptor_texture_and_sampler]
    };

    // Create GPUPipelineLayout type
    const pipelineLayout = device.createPipelineLayout(pipelineLayoutDescriptor);

    if(pipelineLayout == null)
    {
        console.log("initialize() Creating GPUPipelineLayout failed\n");
        throw new Error("initialize() Creating GPUPipelineLayout failed\n");
        return;
    }   

    // Interleaved buffer attributes (PCNT)        
    // create GPUVertexBufferAttribute type for the vertex position buffer
    const positionVertexBufferAttribute = {
        shaderLocation: 0, // This matches with the 0th @location(0) in the vertex shader code
        offset: 0,
        format: "float32x3" // vec3<f32> is represented as float32x3 in WebGPU for R32G32B32 format
    };    

    const colorVertexAttribute = {
        shaderLocation: 1,
        offset: Float32Array.BYTES_PER_ELEMENT * 3, // 12
        format: "float32x3"
    };    

    const normalVertexBufferAttribute = {
        shaderLocation: 2, // This matches with the 1st @location(1) in the vertex shader code
        offset: Float32Array.BYTES_PER_ELEMENT * 6, // Offset is after the color data = 24
        format: "float32x3" // vec3<f32> is represented as float32x3 in WebGPU for R32G32B32 format
    };

    const texcoordVertexBufferLayout = {
        shaderLocation: 3,
        offset: Float32Array.BYTES_PER_ELEMENT * 9,
        format: "float32x2" // vec2<f32>
    };

    // create GPUVertexBufferLayout type for the vertex position buffer
    const interleavedVertexBufferLayout = {
        arrayStride: Float32Array.BYTES_PER_ELEMENT * 11, // 4 * 11 floats = 44 bytes per vertex
        attributes: [positionVertexBufferAttribute,
                        colorVertexAttribute,
                        normalVertexBufferAttribute,
                        texcoordVertexBufferLayout
        ],
        stepMode: "vertex"  // jump vertex by vertex, not instance by instance
    };

    // Create GPUVertexState type for the vertex shader stage
    const vertexShaderState = {
        module: shaderModule_vertex,
        entryPoint: "main",
        buffers: [interleavedVertexBufferLayout]
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

    // step 9 Set clear color to blue
    clear_color = {r: 0.0, g: 0.0, b: 0.0, a: 1.0};
}

// Create vertex buffer UDF
async function createVertexBuffer(_vertexData) {
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

function resize() {

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
    const rotationMatrix_X = mat4.create();
    const scaleMatrix = mat4.create();
    const rotationMatrix_Y = mat4.create();
    const rotationMatrix_Z = mat4.create();
    const rotationMatrix = mat4.create();

    //scaleMatrix = mat4.scale();

    rotationMatrix_Y = mat4.rotateY(modelMatrix, modelMatrix, deg2rad(angleCube));
    rotationMatrix_X = mat4.rotateX(modelMatrix, modelMatrix, deg2rad(angleCube));
    rotationMatrix_Z = mat4.rotateZ(modelMatrix, modelMatrix, deg2rad(angleCube));
    rotationMatrix = mat4.multiply(rotationMatrix_Y, rotationMatrix_X, rotationMatrix_Z);
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -6.0]);
    mat4.multiply(rotationMatrix, projectionMatrix, modelMatrix, viewMatrix);

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

    // 3: Material Ambient, Diffuse, Specular and Shininess
    queue.writeBuffer(buffer_uniform,
        Float32Array.BYTES_PER_ELEMENT * 16  * 3 + Float32Array.BYTES_PER_ELEMENT * 4 * 4, // materialAmbient from 256th byte offset to 271st byte offset
        materialAmbient, 
        0, 
        materialAmbient.length);
    
    queue.writeBuffer(buffer_uniform,
        Float32Array.BYTES_PER_ELEMENT * 16  * 3 + Float32Array.BYTES_PER_ELEMENT * 4 * 5, // materialDiffuse from 272nd byte offset to 287th byte offset
        materialDiffuse, 
        0, 
        materialDiffuse.length);

    queue.writeBuffer(buffer_uniform,
        Float32Array.BYTES_PER_ELEMENT * 16  * 3 + Float32Array.BYTES_PER_ELEMENT * 4 * 6, // materialSpecular from 288th byte offset to 303rd byte offset
        materialSpecular, 
        0, 
        materialSpecular.length);
    
    queue.writeBuffer(buffer_uniform,
        Float32Array.BYTES_PER_ELEMENT * 16  * 3 + Float32Array.BYTES_PER_ELEMENT * 4 * 7, // materialShininess from 304th byte offset to 319th byte offset
        materialShininess, 
        0, 
        materialShininess.length);

    // Step 13 create render pass encoder to record commands for the GPU
    const render_pass_encoder = command_encoder.beginRenderPass(render_pass_descriptor);

    render_pass_encoder.setPipeline(render_pipeline);
    render_pass_encoder.setViewport(0, 0, canvas.width, canvas.height, 0, 1);   // Viewport and scissor rect are same in WebGPU, so we can set viewport only do not have any GPU structure
    render_pass_encoder.setScissorRect(0, 0, canvas.width, canvas.height);
    render_pass_encoder.setBindGroup(0, bindingGroups_uniform);
    render_pass_encoder.setBindGroup(1, bind_group_texture_and_sampler);
    render_pass_encoder.setVertexBuffer(0, buffer_interleaved);

    render_pass_encoder.draw(36); // 6 vertices, 1 instance, first vertex = 0, first instance = 0 draw(6, 1, 0, 0) analogus to glDrawArrays(GL_TRIANGLES, 0, 6) in OpenGL
    
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
    angleCube = angleCube + 1.0; // Increment the rotation angle
    if (angleCube >= 360.0)
        angleCube = 0.0;
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
    
    // Destroy the texture and sampler for the square
   if(texture_marble != null)
   {
       texture_marble.destroy();
       texture_marble = null;
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

        render_pipeline = null;
        buffer_uniform = null;
        bindingGroups_uniform = null;  
        
        sampler_marble = null;
        bind_group_texture_and_sampler = null;
    }

    perspectiveProjectionMatrix = null;

    console.log("WebGPU uninitialization done successfully\n");
}   

// User defined functions
async function loadTexture(_imageFileName) {
    // Code
    const image = new Image(); 
    image.src = _imageFileName;
    await image.decode(); // Wait for the image to be decoded

    const imageBitmap = await createImageBitmap(image);
    if(imageBitmap == null)
    {
        console.log("loadTexture() Creating ImageBitmap failed\n");
        throw new Error("loadTexture() Creating ImageBitmap failed\n");
        return null;
    }
    else
    {
        console.log("loadTexture() Creating ImageBitmap succeeded\n");
    }
    // Create GPUTextureDescriptor type for the texture
    const textureDescriptor = {
        size: [imageBitmap.width, imageBitmap.height, 1],
        dimension: "2d",
        format: "rgba8unorm",
        usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST | GPUTextureUsage.RENDER_ATTACHMENT
    };
    if(textureDescriptor == null)
    {
        console.log("loadTexture() Creating GPUTextureDescriptor failed\n");
        throw new Error("loadTexture() Creating GPUTextureDescriptor failed\n");
        return null;
    }

    const _texture = device.createTexture(textureDescriptor);
    if(_texture == null)
    {
        console.log("loadTexture() Creating createTexture() failed\n");
        throw new Error("loadTexture() Creating createTexture() failed\n");
        return null;
    }

    // Copy the image bitmap to the GPU texture
    const copySource = {
        source : imageBitmap
    };

    const copyDestination = {
        texture: _texture,
        mipLevel: 0
    };

    queue.copyExternalImageToTexture(copySource, copyDestination, textureDescriptor.size);

    return _texture;
}

function createBindGroupLayoutForTextureAndSampler(
    _textureSampleType,
     _textureViewDimension, 
     _isTextureMultisampled, 
     _textureBindingIndex, 
     _textureShaderStageVisibility, 
     _filtering,
     _samplerBindingIndex, 
     _samplerShaderStageVisibility) {
        // Code
        // Create binding layout
        const bindingLayout_texture = {
            sampleType: _textureSampleType,
            viewDimension: _textureViewDimension,
            multisampled: _isTextureMultisampled
        };

        // create bind group layout entry
        const bindGroupLayoutEntry_texture = {
            binding: _textureBindingIndex,
            visibility: _textureShaderStageVisibility,
            texture: bindingLayout_texture
        };

        // Create binding layout for the sampler
        const bindingLayout_sampler = {
            type: _filtering
        };

        // create bind group layout entry for the sampler
        const bindGroupLayoutEntry_sampler = {
            binding: _samplerBindingIndex,
            visibility: _samplerShaderStageVisibility,
            sampler: bindingLayout_sampler
        };

        // Create the bind group layout descriptor
        const bindGroupLayoutDescriptor = {
            entries: [bindGroupLayoutEntry_texture, bindGroupLayoutEntry_sampler]
        };

        // Create the bind group layout
        const bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDescriptor);

        if(bindGroupLayout == null) {
            console.log("createBindGroupLayoutForTextureAndSampler() Creating bind group layout failed\n");
            throw new Error("createBindGroupLayoutForTextureAndSampler() Creating bind group layout failed\n");
            return null;
        }

        return bindGroupLayout;
}

function createBindGroupForTextureAndSampler(_textureBindingIndex, _texture, _samplerBindingIndex, _sampler, _bindGroupLayout) {

    // Code
    // Create the bind group entry for the texture
    const bindGroupEntry_texture = {
        binding: _textureBindingIndex,
        resource: _texture.createView()
    };

    // Create the bind group entry for the sampler
    const bindGroupEntry_sampler = {
        binding: _samplerBindingIndex,
        resource: _sampler
    };

    // Create the bind group descriptor
    const bindGroupDescriptor = {
        layout: _bindGroupLayout,
        entries: [bindGroupEntry_texture, bindGroupEntry_sampler]
    };

    // Create the bind group
    const bindGroup = device.createBindGroup(bindGroupDescriptor);

    if(bindGroup == null) {
        console.log("createBindGroupForTextureAndSampler() Creating bind group failed\n");
        throw new Error("createBindGroupForTextureAndSampler() Creating bind group failed\n");
        return null;
    }

    return bindGroup;
}

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
