//-----------------------------------------------------------------------------
//  [PGR2] Tile-base Deffered Shading
//  12/02/2020
//-----------------------------------------------------------------------------
const char* help_message =
"[PGR2] Tile-based Deffered Shading\n\
-------------------------------------------------------------------------------\n\
CONTROLS: \n\
   [M/m]   ... inc/dec number of cube models\n\
   [L/l]   ... inc/dec number of lights\n\
   [R/r]   ... inc/dec maximal radius of lights\n\
   [S/s]   ... inc/dec side size of a super-pixel\n\
   [Z/z]   ... move scene along z-axis\n\
   [d]     ... enable/disable tile based deferred shading\n\
   [t]     ... enable/disable rendering light low res super-pixels\n\
   [c]     ... compile shaders\n\
   [mouse] ... scene rotation (left button)\n\
-------------------------------------------------------------------------------";

// IMPLEMENTATION______________________________________________________________

//-----------------------------------------------------------------------------
// Name: compileShaders()
// Desc: 
//-----------------------------------------------------------------------------
void TW_CALL compileShaders(void *clientData) {
    // Create shader program object
    Tools::Shader::CreateShaderProgramFromFile(naiveRenderingProgram, "standard_rendering.vs", nullptr, nullptr, nullptr, "standard_rendering.fs");
	Tools::Shader::CreateShaderProgramFromFile(geometryPassProgram, "geometry_pass.vs", nullptr, nullptr, nullptr, "geometry_pass.fs");
	Tools::Shader::CreateShaderProgramFromFile(lightIDSPixelProgram, "sup_pixel_lightIDs.vs", nullptr, nullptr, nullptr, "sup_pixel_lightIDs.fs");
	Tools::Shader::CreateShaderProgramFromFile(lowResRenderingProgram, "render_low_res.vs", nullptr, nullptr, nullptr, "render_low_res.fs");
	Tools::Shader::CreateShaderProgramFromFile(finalRenderingProgram, "final_rendering.vs", nullptr, nullptr, nullptr, "final_rendering.fs");
}


//-----------------------------------------------------------------------------
// Name: initGUI()
// Desc: 
//-----------------------------------------------------------------------------
void initGUI(TwBar* menu) {
#ifdef USE_ANTTWEAKBAR
    TwDefine(" Controls position='10 10' size='240 370' valueswidth=70 refresh=0.1 ");
    TwAddVarRW(menu, "num_models_in_row", TW_TYPE_INT32, &sceneNumCubes, " group='Scene' label='num models in row' min=1 max=100 step=1 keyIncr=M keyDecr=m help='Number of models in row.' ");
	TwAddVarRW(menu, "num_lights", TW_TYPE_INT32, &sceneNumLights, " group='Scene' label='num lights' min=1 max=512 step=1 keyIncr=L keyDecr=l help='Number of lights in the scene.' ");
	TwAddVarRW(menu, "max_light_radius", TW_TYPE_FLOAT, &maxLightRadius, " group='Scene' label='max light radius' min=1.0f max=50.0f step=1.0f keyIncr=R keyDecr=r help='Maximal radius of lights.' ");
	TwAddVarRW(menu, "deferred_shading", TW_TYPE_BOOLCPP, &modeDefferedShading, " group='Render' label='deferred shading' key=d help='Toggle deffered shading mode.' ");
	TwAddVarRW(menu, "ligh_super_pixels", TW_TYPE_BOOLCPP, &modeLightSuperPixelsOnly, " group='Render' label='light super-pixels' key=t help='Show light low res super-pixels.' ");
	TwAddVarRW(menu, "suppix_side_size", TW_TYPE_INT32, &superPixelSideSize, " group='Render' label='super-pixel side size' min=1 max=128 step=1 keyIncr=S keyDecr=s help='Change super-pixel side size.' ");
#endif
}


//-----------------------------------------------------------------------------
// Name: keyboardChanged()
// Desc: 
//-----------------------------------------------------------------------------
void keyboardChanged(int key, int action, int mods) {
#ifndef USE_ANTTWEAKBAR
#endif
}


//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    int OGL_CONFIGURATION[] = {
        GLFW_CONTEXT_VERSION_MAJOR, 4,
        GLFW_CONTEXT_VERSION_MINOR, 0,
        GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE,
        GLFW_OPENGL_DEBUG_CONTEXT,  GL_TRUE,
        GLFW_OPENGL_PROFILE,        GLFW_OPENGL_COMPAT_PROFILE, // GLFW_OPENGL_CORE_PROFILE
        0
    };

    printf("%s\n", help_message);

    return common_main(800, 600, "[PGR2] Tile-based Deffered Shading",
                       OGL_CONFIGURATION, // OGL configuration hints
                       initGL,            // Init GL callback function
                       initGUI,           // Init GUI callback function
                       display,           // Display callback function
                       nullptr,           // Window resize callback function
                       keyboardChanged,   // Keyboard callback function
                       nullptr,           // Mouse button callback function
                       nullptr);          // Mouse motion callback function
}