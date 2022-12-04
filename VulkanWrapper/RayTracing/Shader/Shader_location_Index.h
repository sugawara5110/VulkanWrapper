//*****************************************************************************************//
//**                             Shader_location_Index.h                                 **//
//*****************************************************************************************//

char* Shader_location_Index_Clo_0 =

"const int rayPayloadInEXT_location = 0;\n"
"const int rayPayloadEXT_location = 1;\n"
"const int cloMiss_Index = 1;\n"
"const int emMiss_Index = 2;\n";

char* Shader_location_Index_Clo_1 =

"const int rayPayloadInEXT_location = 1;\n"
"const int rayPayloadEXT_location = 0;\n"
"const int cloMiss_Index = 0;\n"
"const int emMiss_Index = 3;\n";

char* Shader_location_Index_In_0 =

"const int rayPayloadInEXT_location = 0;\n";

char* Shader_location_Index_In_1 =

"const int rayPayloadInEXT_location = 1;\n";

char* Shader_location_Index_In_0_Miss =

"#version 460\n"
"const int rayPayloadInEXT_location = 0;\n";

char* Shader_location_Index_In_1_Miss =

"#version 460\n"
"const int rayPayloadInEXT_location = 1;\n";