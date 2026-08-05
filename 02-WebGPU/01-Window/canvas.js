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
    console.log("Configuring WebGPU context successfully done with format: canvas_format\n");

    // step 9 Set clear color to blue
    clear_color = {r: 0.0, g: 0.0, b: 1.0, a: 1.0};
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
    }

    console.log("WebGPU uninitialization done successfully\n");
}   

