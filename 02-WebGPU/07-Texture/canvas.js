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
 
let buffer_position_triangle = null;
let buffer_position_square = null;
let buffer_texcoords_square = null;
let buffer_mvpUniform_triangle = null;
let buffer_mvpUniform_square = null;
let render_pipeline = null;
let bindingGroups_mvpUniform_triangle = null;
let bindingGroups_mvpUniform_square = null;
let perspectiveProjectionMatrix = null;

let texture_smiley = null;
let sampler_smiley = null;
let bind_group_texture_and_sampler = null;
 
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
    await initialize(); // This makes use of async-await to wait for GPU to be ready and then call initialize() function to initialize WebGPU
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
 
    buffer_position_triangle = null;
    buffer_position_square = null;
    buffer_texcoords_square = null;
    render_pipeline = null;
    buffer_mvpUniform_triangle = null;
    buffer_mvpUniform_square = null;

    texture_smiley = null;
    sampler_smiley = null;
    bind_group_texture_and_sampler = null;

    bindingGroups_mvpUniform_triangle = null;
    bindingGroups_mvpUniform_square = null;
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
    console.log(`initialize() Configuring WebGPU context successfully done with format: ${canvas_format}\n`);
 
    // Vertex shader code source in WGSL (WebGPU Shading Language)
    const vertexShaderSourceCode = "struct MVPUniform\n" +
    "{\n" +
        "mvpMatrix : mat4x4<f32>\n" +
    "};\n" +
    "struct VertexOutput\n" +
    "{\n" +
        "@builtin(position) position: vec4<f32>,\n" +
        "@location(0) texcoords: vec2<f32>\n" +
    "};\n" +
    "@group(0) @binding(0) var<uniform> mvpUniform : MVPUniform;\n" +
    "@vertex\n" +
    "fn main(@location(0) pos : vec4<f32>, @location(1) tex : vec2<f32>) -> VertexOutput\n"+
    "{\n"+
     "var output: VertexOutput;\n"+
     "output.position = mvpUniform.mvpMatrix * pos;\n" +
     "output.texcoords = tex;\n" +
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
        "@location(0) texcoords: vec2<f32>\n" +
    "};\n" +
    "@group(1) @binding(0) var myTexture2D : texture_2d<f32>;\n" +
    "@group(1) @binding(1) var mySampler : sampler;\n" +
    "@fragment\n" +
    "fn main(output: VertexOutput) -> @location(0) vec4<f32>\n" +
    "{\n" +
        "var colors = textureSample(myTexture2D, mySampler, output.texcoords);\n" + // Active texture sampling using textureSample() function, which takes texture, sampler and texture coordinates as input and returns the sampled color
        "return colors;\n" + // return sampled texture color
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
 
    const vertex_position_triangle = new Float32Array([
        0.0, 1.0, 0.0, 1.0,// Apex
        -1.0, -1.0, 0.0, 1.0, // Left bottom
        1.0, -1.0, 0.0, 1.0 // Right bottom
    ]);
 
    const vertex_position_square = new Float32Array([
        1.0, 1.0, 0.0, 1.0, // Right top
        -1.0, 1.0, 0.0, 1.0,  // Left top
        -1.0, -1.0, 0.0, 1.0, // Left bottom
 
        -1.0, -1.0, 0.0, 1.0, // Left bottom
        1.0, -1.0, 0.0, 1.0,  // Right bottom
        1.0, 1.0, 0.0, 1.0 // Right top
    ]);
 
    const vertex_texcoords_square = new Float32Array([
        1.0, 1.0, 
        0.0, 1.0, 
        0.0, 0.0, 
 
        0.0, 0.0, 
        1.0, 0.0, 
        1.0, 1.0 
    ]);
 
    // Bind group layout enter GPUBindGroupLayoutEntry type is common for both triangle and square, so we can use same bind group layout for both
    const bindGroupLayout_mvpUniform = createBindGroupLayoutForUniform(0, GPUShaderStage.VERTEX, "uniform");
 
    // Triangle: int positionBuffer, int bindGroup, int uniformBuffer;
    // Triangle position buffer
    const bufferDescriptor_position_triangle = {
        size: vertex_position_triangle.byteLength,
        usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
    };
 
    buffer_position_triangle = device.createBuffer(bufferDescriptor_position_triangle);
 
    if(buffer_position_triangle == null)
    {
        console.log("initialize() Creating GPU buffer for vertex positions triangle failed\n");
        throw new Error("initialize() Creating GPU buffer for vertex positions triangle failed\n");
        return;
    }
 
    // Copy the vertex positions data to the GPU buffer
    queue.writeBuffer(buffer_position_triangle, 0, vertex_position_triangle, 0, vertex_position_triangle.length);
    console.log("initialize() Creating GPU buffer for vertex positions triangle successfully done\n");
 
    const mvpUniformBufferSize = 16 * 4; // 16 floats * 4 bytes per float
    buffer_mvpUniform_triangle = createUniformBuffer(mvpUniformBufferSize, GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST);
 
    // create bind group for mvp uniform for triangle
    bindingGroups_mvpUniform_triangle = createBindGroupForUniform(buffer_mvpUniform_triangle, 0, mvpUniformBufferSize, 0, bindGroupLayout_mvpUniform);
 
    // Square position buffer
    const bufferDescriptor_position_square = {
        size: vertex_position_square.byteLength,
        usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
    };
    buffer_position_square = device.createBuffer(bufferDescriptor_position_square);
 
    if(buffer_position_square == null)
    {
        console.log("initialize() Creating GPU buffer for vertex positions square failed\n");
        throw new Error("initialize() Creating GPU buffer for vertex positions square failed\n");
        return;
    }
 
    // Copy the vertex positions data to the GPU buffer
    queue.writeBuffer(buffer_position_square, 0, vertex_position_square, 0, vertex_position_square.length);
    console.log("initialize() Creating GPU buffer for vertex positions square successfully done\n");
 
    // Create GPU buffer for vertex texture coordinates GPUBufferDescriptor type
    const bufferDescriptor_texcoords_square = {
        size: vertex_texcoords_square.byteLength,
        usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
    };
 
    // Create the GPU buffer for vertex texture coordinates GPUBuffer type
    buffer_texcoords_square = device.createBuffer(bufferDescriptor_texcoords_square);
 
    if(buffer_texcoords_square == null)
    {
        console.log("initialize() Creating GPU buffer for vertex texture coordinates for square failed\n");
        throw new Error("initialize() Creating GPU buffer for vertex texture coordinates for square failed\n");
        return;
    }
 
    // Copy the vertex texture coordinates data to the GPU buffer
    queue.writeBuffer(buffer_texcoords_square, 0, vertex_texcoords_square, 0, vertex_texcoords_square.length);
    console.log("initialize() Creating GPU buffer for vertex texture coordinates square successfully done\n");
 
    buffer_mvpUniform_square = createUniformBuffer(mvpUniformBufferSize, GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST);
    // create bind group for mvp uniform for square
    bindingGroups_mvpUniform_square = createBindGroupForUniform(buffer_mvpUniform_square, 0, mvpUniformBufferSize, 0, bindGroupLayout_mvpUniform);

    // Load image and create texture and sampler for the square
    texture_smiley = await loadTexture("Smiley.png");
    if(texture_smiley == null)
    {
        console.log("initialize() Loading texture for square failed\n");
        throw new Error("initialize() Loading texture for square failed\n");
        return;
    }
    else
    {
        console.log("initialize() Loading texture for square succeeded\n");
    }

    const samplerDescriptor = {
        magFilter: "linear",
        minFilter: "linear"
    };

    // Create texture sampler
    sampler_smiley = device.createSampler(samplerDescriptor);
    if(sampler_smiley == null)
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
        texture_smiley,
        1,
        sampler_smiley, 
        bindGroupLayoutDescriptor_texture_and_sampler
    );

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
 
    // create GPUVertexBufferAttribute type for the vertex texture coordinates buffer
    const texcoordVertexBufferAttribute = {
        shaderLocation: 1, // This matches with the 1st @location(1) in the vertex shader code
        offset: 0,
        format: "float32x2" // vec2<f32> is represented as float32x2 in WebGPU for R32G32 format
    };
 
    // create GPUVertexBufferLayout type for the vertex texture coordinates buffer
    const texcoordVertexBufferLayout = {
        arrayStride: 4 * 2, // 4 floats * 2 bytes per float
        attributes: [texcoordVertexBufferAttribute],
        stepMode: "vertex"  // jump vertex by vertex, not instance by instance
    };

    // Create GPUVertexState type for the vertex shader stage
    const vertexShaderState = {
        module: shaderModule_vertex,
        entryPoint: "main",
        buffers: [positionVertexBufferLayout, texcoordVertexBufferLayout]
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

 // 3 UDF for texture and sampler bind group layout and bind group creation
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

 

    // Step 13 create render pass encoder to record commands for the GPU

    const render_pass_encoder = command_encoder.beginRenderPass(render_pass_descriptor);

 

    render_pass_encoder.setPipeline(render_pipeline);

    render_pass_encoder.setViewport(0, 0, canvas.width, canvas.height, 0, 1);   // Viewport and scissor rect are same in WebGPU, so we can set viewport only do not have any GPU structure

    render_pass_encoder.setScissorRect(0, 0, canvas.width, canvas.height);

 

    // Triangle

    var mvpMatrix = mat4.create();

    /*

    var modelViewMatrix = mat4.create();

    mat4.translate(modelViewMatrix, modelViewMatrix, [-1.5, 0.0, -6.0]);

    mat4.multiply(mvpMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    queue.writeBuffer(buffer_mvpUniform_triangle, 0, mvpMatrix, 0, mvpMatrix.length);

    render_pass_encoder.setVertexBuffer(0, buffer_position_triangle);

    render_pass_encoder.setBindGroup(0, bindingGroups_mvpUniform_triangle);

    render_pass_encoder.draw(3); // 3 vertices, 1 instance, first vertex = 0, first instance = 0 draw(3, 1, 0, 0) analogus to glDrawArrays(GL_TRIANGLES, 0, 3) in OpenGL

    */

 

    // Square
    mvpMatrix = mat4.create();
    modelViewMatrix = mat4.create();
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -5.0]);
    mat4.multiply(mvpMatrix, perspectiveProjectionMatrix, modelViewMatrix);
    queue.writeBuffer(buffer_mvpUniform_square, 0, mvpMatrix, 0, mvpMatrix.length);

    render_pass_encoder.setVertexBuffer(0, buffer_position_square);
    render_pass_encoder.setVertexBuffer(1, buffer_texture_square);

    render_pass_encoder.setBindGroup(0, bindingGroups_mvpUniform_square);
    render_pass_encoder.setBindGroup(1, bind_group_texture_and_sampler);

    render_pass_encoder.draw(6); // 6 vertices, 1 instance, first vertex = 0, first instance = 0 draw(6, 1, 0, 0) analogus to glDrawArrays(GL_TRIANGLES, 0, 6) in OpenGL


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
 
   // Destroy the texture and sampler for the square
   if(texture_smiley != null)
   {
       texture_smiley.destroy();
       texture_smiley = null;
   }
   if(sampler_smiley != null)
   {
       sampler_smiley.destroy();
       sampler_smiley = null;
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
        buffer_texture_square = null;
        buffer_position_square = null;
        buffer_position_triangle = null;
        render_pipeline = null;
        buffer_mvpUniform_square = null;
        buffer_mvpUniform_triangle = null;
        bindingGroups_mvpUniform_square = null;
        bindingGroups_mvpUniform_triangle = null;

        sampler_smiley = null;
        bind_group_texture_and_sampler = null;
    }
 
    perspectiveProjectionMatrix = null;
    console.log("WebGPU uninitialization done successfully\n");
}
