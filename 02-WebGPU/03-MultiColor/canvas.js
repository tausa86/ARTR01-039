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

let buffer_position = null;
let buffer_colors = null;
let render_pipeline = null;
let buffer_mvpUniform = null;
let bindingGroups_mvpUniform = null;
let perspectiveProjectionMatrix = null;

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

    buffer_position = null;
    buffer_colors = null;
    render_pipeline = null;
    buffer_mvpUniform = null;
    bindingGroups_mvpUniform = null;
    perspectiveProjectionMatrix = null;
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
    const vertexShaderSourceCode = "struct MVPUniform\n" +
    "{\n" +
        "mvpMatrix : mat4x4<f32>\n" +
    "};\n" +
    "struct VertexOutput\n" +
    "{\n" +
        "@builtin(position) position: vec4<f32>,\n" +
        "@location(0) color: vec4<f32>\n" +
    "};\n" +
    "@group(0) @binding(0) var<uniform> mvpUniform : MVPUniform;\n" +
    "@vertex\n" +
    "fn main(@location(0) pos : vec4<f32>, @location(1) col : vec4<f32>) -> VertexOutput\n"+
    "{\n"+
    "var output: VertexOutput;\n"+
     "output.position = mvpUniform.mvpMatrix * pos;\n" + 
     "return output;\n" +
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
    const fragmentShaderSourceCode = "struct VertexOutput\n" +
    "{\n" +
        "@builtin(position) position: vec4<f32>,\n" +
        "@location(0) color: vec4<f32>\n" +
    "};\n" +
    "@fragment\n" +
    "fn main(output: VertexOutput) -> @location(0) vec4<f32>\n" +
    "{\n" +
        "return output.color;\n" + // return color
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

    const vertex_position = new Float32Array([
        0.0, 1.0, 0.0, 1.0,// Apex
        -1.0, -1.0, 0.0, 1.0, // Left bottom
        1.0, -1.0, 0.0, 1.0 // Right bottom
    ]);

     const vertex_color = new Float32Array([
        1.0, 0.0, 0.0, 1.0,// Red color for Apex
        0.0, 1.0, 0.0, 1.0, // Green color for Left bottom
        0.0, 0.0, 1.0, 1.0 // Blue color for Right bottom
    ]);

    // Create GPU buffer for vertex positions GPUBufferDescriptor type
    const bufferDescriptor_position = {
        size: vertex_position.byteLength,
        usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
    };

    // Create the GPU buffer for vertex positions GPUBuffer type
    buffer_position = device.createBuffer(bufferDescriptor_position);

    if(buffer_position == null)
    {
        console.log("initialize() Creating GPU buffer for vertex positions failed\n");
        throw new Error("initialize() Creating GPU buffer for vertex positions failed\n");
        return;
    }

    // Copy the vertex positions data to the GPU buffer
    queue.writeBuffer(buffer_position, 0, vertex_position, 0, vertex_position.length);
    console.log("initialize() Creating GPU buffer for vertex positions successfully done\n");

    // Create GPU buffer for vertex colors GPUBufferDescriptor type
    const bufferDescriptor_color = {
        size: vertex_color.byteLength,
        usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
    };

    // Create the GPU buffer for vertex colors GPUBuffer type
    buffer_color = device.createBuffer(bufferDescriptor_color);

    if(buffer_color == null)
    {
        console.log("initialize() Creating GPU buffer for vertex colors failed\n");
        throw new Error("initialize() Creating GPU buffer for vertex colors failed\n");
        return;
    }

    // Copy the vertex colors data to the GPU buffer
    queue.writeBuffer(buffer_color, 0, vertex_color, 0, vertex_color.length);
    console.log("initialize() Creating GPU buffer for vertex colors successfully done\n");

    // Uniform Plumbing
    // Bind group layout enter GPUBindGroupLayoutEntry type
    const bindGroupLayoutEntry_mvpUniform = {
        binding: 0, // This matches with the 0th group(0) @binding(0) in the vertex shader code
        visibility: GPUShaderStage.VERTEX,
        buffer: {type: "uniform"}
    };

    // Create GPUBindGroupLayoutDescriptor type
    const bindGroupLayoutDescriptor = {
        entries: [bindGroupLayoutEntry_mvpUniform]
    };

    // Create GPUBindGroupLayout type
    const bindGroupLayout_mvpUniform = device.createBindGroupLayout(bindGroupLayoutDescriptor);

    if(bindGroupLayout_mvpUniform == null)
    {
        console.log("initialize() Creating GPUBindGroupLayout for MVP uniform failed\n");
        throw new Error("initialize() Creating GPUBindGroupLayout for MVP uniform failed\n");
        return;
    }

    // Create GPUPipelineLayoutDescriptor type
    const pipelineLayoutDescriptor = {
        bindGroupLayouts: [bindGroupLayout_mvpUniform]
    };

    // Create GPUPipelineLayout type
    const pipelineLayout = device.createPipelineLayout(pipelineLayoutDescriptor);

    if(pipelineLayout == null)
    {
        console.log("initialize() Creating GPUPipelineLayout failed\n");
        throw new Error("initialize() Creating GPUPipelineLayout failed\n");
        return;
    }

    const mvpUniformBufferSize = 16 * 4; // 16 floats * 4 bytes per float
    const bufferDescriptor_mvpUniform = {
        size: mvpUniformBufferSize,
        usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST
    };

    buffer_mvpUniform = device.createBuffer(bufferDescriptor_mvpUniform);
    if(buffer_mvpUniform == null)
    {
        console.log("initialize() Creating GPU buffer for MVP uniform failed\n");
        throw new Error("initialize() Creating GPU buffer for MVP uniform failed\n");
        return;
    }

    // Create GPUBufferBinding type for the MVP uniform buffer
    const bufferBinding_mvpUniform = {
        buffer: buffer_mvpUniform,
        offset: 0,
        size: mvpUniformBufferSize
    };
    
    // Create GPUBindGroupEntry type for the MVP uniform buffer
    const bindGroupEntry_mvpUniform = {
        binding: 0, // This matches with the 0th group(0) the vertex shader code
        resource: bufferBinding_mvpUniform
    };

    // Create GPUBindGroupDescriptor type
    const bindGroupDescriptor_mvpUniform = {
        layout: bindGroupLayout_mvpUniform,
        entries: [bindGroupEntry_mvpUniform]
    };

    // Create GPUBindGroup type for the MVP uniform buffer
    bindingGroups_mvpUniform = device.createBindGroup(bindGroupDescriptor_mvpUniform);  
    if(bindingGroups_mvpUniform == null)
    {
        console.log("initialize() Creating GPUBindGroup for MVP uniform failed\n");
        throw new Error("initialize() Creating GPUBindGroup for MVP uniform failed\n");
        return;
    }

    // create GPUVertexBufferAttribute type for the vertex position buffer
    const positionVertexBufferAttribute = {
        shaderLocation: 0, // This matches with the 0th @location(0) in the vertex shader code
        offset: 0,
        format: "float32x4" // vec4<f32> is represented as float32x4 in WebGPU for R32G32B32A32 format
    };

    // create GPUVertexBufferLayout type for the vertex position buffer
    const positionVertexBufferLayout = {
        arrayStride: 4 * 4, // 4 floats * 4 bytes per float
        attributes: [positionVertexBufferAttribute],
        stepMode: "vertex"  // jump vertex by vertex, not instance by instance
    };

    // create GPUVertexBufferAttribute type for the vertex color buffer
    const colorVertexBufferAttribute = {
        shaderLocation: 1, // This matches with the 1st @location(1) in the vertex shader code
        offset: 0,
        format: "float32x4" // vec4<f32> is represented as float32x4 in WebGPU for R32G32B32A32 format
    };

    // create GPUVertexBufferLayout type for the vertex color buffer
    const colorVertexBufferLayout = {
        arrayStride: 4 * 4, // 4 floats * 4 bytes per float
        attributes: [colorVertexBufferAttribute],
        stepMode: "vertex"  // jump vertex by vertex, not instance by instance
    };

    // Create GPUVertexState type for the vertex shader stage
    const vertexShaderState = {
        module: shaderModule_vertex,
        entryPoint: "main",
        buffers: [positionVertexBufferLayout, colorVertexBufferLayout]
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
        frontFace: "cw",   // ccw = counter-clockwise winding order for front face, cw = clockwise winding order for front face
        cullMode: "none"    // none = no culling, front = cull front face, back = cull back face
    };

    // create render pipeline descriptor of GPURenderPipelineDescriptor type
    const renderPipelineDescriptor = {
        layout: pipelineLayout,
        vertex: vertexShaderState,
        fragment: fragmentShaderState,
        primitive: primitiveState
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

function resize(){

    // Code
    if(bFullScreen){
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    } else {
        canvas.width = canvas_original_width;
        canvas.height = canvas_original_height;
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

    // Step 12 create render pass descriptor to begin render pass of GPURenderPassDescriptor type
    const render_pass_descriptor = {
        colorAttachments: [render_pass_color_attachment]
    };

    const mvpMatrix = mat4.create();
    const modelViewMatrix = mat4.create();
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -5.0]);
    mat4.multiply(mvpMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    queue.writeBuffer(buffer_mvpUniform, 0, mvpMatrix, 0, mvpMatrix.length);

    // Step 13 create render pass encoder to record commands for the GPU
    const render_pass_encoder = command_encoder.beginRenderPass(render_pass_descriptor);

    render_pass_encoder.setPipeline(render_pipeline);
    render_pass_encoder.setViewport(0, 0, canvas.width, canvas.height, 0, 1);   // Viewport and scissor rect are same in WebGPU, so we can set viewport only do not have any GPU structure
    render_pass_encoder.setScissorRect(0, 0, canvas.width, canvas.height);
    render_pass_encoder.setBindGroup(0, bindingGroups_mvpUniform);
    render_pass_encoder.setVertexBuffer(0, buffer_position);
    render_pass_encoder.setVertexBuffer(1, buffer_color);
    render_pass_encoder.draw(3); // 3 vertices, 1 instance, first vertex = 0, first instance = 0

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

    // Step 18 Unconfigure & Destroy the context
    if(null != context)
    {
        context.unconfigure();
        context = null;
    }

    // Step 19 Destroy the device
    if(null != device)
    {
        device.destroy();
        device = null;
        queue = null;
        canvas_format = null;
        buffer_position = null;
        buffer_colors = null;
        render_pipeline = null;
        buffer_mvpUniform = null;
        bindingGroups_mvpUniform = null;        
    }

    perspectiveProjectionMatrix = null;

    console.log("WebGPU uninitialization done successfully\n");
}   

