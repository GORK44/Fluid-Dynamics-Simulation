//
//  main.cpp
//  OpenGLtest1
//
//  Created by apple on 2019/5/3.
//  Copyright © 2019 apple. All rights reserved.
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Camera.hpp"
#include "Model.hpp"

#include "gork.hpp"
#include "Material.hpp"




//摄像机
Camera GORK::camera(glm::vec3(0.0f, 0.0f, 2.0f));




int main()
{


    // 创建一个窗口对象 glfw window creation
    GLFWwindow* window = GORK::InitWindow();
    if (!window) return -1;


    // ----------------------------------
    //     //鼠标控制
    glfwSetCursorPosCallback(window, GORK::Mouse_callback);//鼠标一移动mouse_callback函数就会被调用
    glfwSetScrollCallback(window, GORK::Scroll_callback);//滚轮一滚动scroll_callback函数就会被调用

    //// 捕捉鼠标 tell GLFW to capture our mouse
    //    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // ----------------------------------



    //========================================================================
    // 配置 g-buffer 帧缓冲区 configure g-buffer framebuffer
    // ------------------------------
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer); //创建一个帧缓冲对象
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer); //将它绑定为激活的(Active)帧缓冲，做一些操作，之后解绑帧缓冲。
    unsigned int gPosition, gNormal, gAlbedoSpec; //三张纹理附件
    // 位置颜色缓冲 position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
//    GORK::TextureAttachments(gPosition, 0);

    // 法线颜色缓冲 normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
//    GORK::TextureAttachments(gNormal, 1);

//    // 漫反射颜色+镜面反射 颜色缓冲 color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
//    GORK::TextureAttachments(gAlbedoSpec, 2);

    // 告诉OpenGL我们将使用（此帧缓冲区的）哪些颜色附件进行渲染tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };//传递多个颜色纹理附件的枚举
    glDrawBuffers(3, attachments);//当渲染到这个帧缓冲中的时候，每当着色器使用location(=0或=1或=2)，那么fragment就会用相应的颜色缓冲就会被用来渲染。


    // 深度缓冲：渲染缓冲对象（只写） create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;//写入深度用于测试
//    glGenRenderbuffers(1, &rboDepth);
//    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    GORK::RenderBufferObject(rboDepth);


    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // 照明信息 lighting info
    // -------------
    const unsigned int NR_LIGHTS = 20;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;

    GORK::SetDeferredLightInfo(NR_LIGHTS, lightPositions, lightColors);
//    srand(13);
//    for (unsigned int i = 0; i < NR_LIGHTS; i++)
//    {
//        // 计算轻微的随机偏移 calculate slightly random offsets
//        float xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
//        float yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;
//        float zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
//        lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
//        // 还计算随机颜色 also calculate random color
//        float rColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
//        float gColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
//        float bColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
//        lightColors.push_back(glm::vec3(rColor, gColor, bColor));
//    }
    //========================================================================



    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        
        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };
    
    // screen quad VAO（帧缓冲屏幕四边形）
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));






    //配置全局opengl状态
    //========================================================================

    //启用深度测试
    glEnable(GL_DEPTH_TEST);
    //    glDepthFunc(GL_LESS);
    glDepthFunc(GL_LEQUAL);//如果目标像素z值<＝当前像素z值，则绘制目标像素(天空盒z=1)

    // 在立方体贴图的面之间进行正确过滤
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);//pre-filter map中高粗糙度(低Mip级别)的立方体贴图接缝处过滤


    //启用模版测试
    //    glEnable(GL_STENCIL_TEST);
    //    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    //启用混合
    //    glEnable(GL_BLEND);
    //    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //源因子 等于C¯source的alpha分量，目标因子 等于1− C¯source的alpha分量

    // 启用面剔除
        glEnable(GL_CULL_FACE); // 启用面剔除，默认剔除背向面（默认 正向面：逆时针，背向面：顺时针）
    //    glCullFace(GL_FRONT); //剔除正向面（默认剔除背向面）
    //    glFrontFace(GL_CCW); //将 逆时针 的面定义为正向面【默认 正向面：逆时针，背向面：顺时针】
    //========================================================================





    //构建并编译着色器程序
    //========================================================================
    //Deferred Shading
    Shader shader_GeometryPass("/书/OGL_Test/Shader/G_buffer.vs", "/书/OGL_Test/Shader/G_buffer.fs");
    Shader shader_LightingPass("/书/OGL_Test/Shader/deferred_shading.vs", "/书/OGL_Test/Shader/deferred_shading.fs");
    Shader shader_LightBox("/书/OGL_Test/Shader/deferred_light_box.vs", "/书/OGL_Test/Shader/deferred_light_box.fs");
//    shader_LightingPass.use();
//    shader_LightingPass.setInt("gPosition", 0);
//    shader_LightingPass.setInt("gNormal", 1);
//    shader_LightingPass.setInt("gAlbedoSpec", 2);

    Shader screenShader("/书/OGL_Test/Shader/screen.vs", "/书/OGL_Test/Shader/screen.fs");
    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    
    

    //模型
    Shader modelShader("/书/OGL_Test/Shader/model.vs", "/书/OGL_Test/Shader/model.fs");
    Model nanosuit("/书/OGL_Test/nanosuit/nanosuit.obj");
    //    Shader model_normal_Shader("/书/OGL_Test/Shader/modelVS.vs", "/书/OGL_Test/Shader/modelFS.fs");
    //    Model personN("/书/OGL_Test/nanosuit/nanosuit.obj");
    std::vector<glm::vec3> objectPositions = GORK::DeferredObjectPositions();
//    objectPositions.push_back(glm::vec3(-3.0,  -3.0, -3.0));
//    objectPositions.push_back(glm::vec3( 0.0,  -3.0, -3.0));
//    objectPositions.push_back(glm::vec3( 3.0,  -3.0, -3.0));
//    objectPositions.push_back(glm::vec3(-3.0,  -3.0,  0.0));
//    objectPositions.push_back(glm::vec3( 0.0,  -3.0,  0.0));
//    objectPositions.push_back(glm::vec3( 3.0,  -3.0,  0.0));
//    objectPositions.push_back(glm::vec3(-3.0,  -3.0,  3.0));
//    objectPositions.push_back(glm::vec3( 0.0,  -3.0,  3.0));
//    objectPositions.push_back(glm::vec3( 3.0,  -3.0,  3.0));




    // ------------------------------------------------------------------------------








    // ------------------------------
    // 渲染循环 render loop
    while (!glfwWindowShouldClose(window)) //每次循环的开始前检查一次GLFW是否被要求退出
    {


        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime(); //获取当前时间
        GORK::deltaTime = currentFrame - GORK::lastFrame; //上一帧时间间隔
        GORK::lastFrame = currentFrame; //保存时间，留在下一帧用


        // 输入 input
        GORK::ProcessInput(window); //输入控制（检查是否按下Esc）



        // render
        // -----------------------------------------
        // 第一处理阶段(Pass 1)
        //=========================================================================

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //设置清空屏幕后填充的颜色
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //清空屏幕的 颜色缓冲 和 深度缓冲


        // draw 绘制场景
        // -----------------------------------------
        //        // 配置观察/投影矩阵 configure view/projection matrices
        glm::mat4 projection = glm::perspective(glm::radians(GORK::camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); //投影矩阵（FOV视锥体竖直张开角度，横向竖直张开角度比，近裁剪面深度，远裁剪面深度）
        glm::mat4 view = GORK::camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        glm::vec3 camPos = GORK::camera.Position;

        //===========================================================================================
        // 1. 几何处理阶段：将场景的几何/颜色数据渲染到gbuffer中
        // geometry pass: render scene's geometry/color data into gbuffer
        // -----------------------------------------------------------------

        GORK::gBufferBind(gBuffer);//绑定gBuffer
        // --------

        GORK::ShaderSet(shader_GeometryPass, view, camPos, projection);//设置shader
        GORK::DrawNanosuits(objectPositions, shader_GeometryPass, nanosuit);//绘制模型，将数据渲染到gPosition, gNormal, gAlbedoSpec三张gBuffer纹理附件上

        // --------
        GORK::gBufferUnBind();//解绑gBuffer


        //=========================================================================

        
        
        
        // -----------------------------------------
        // 第二处理阶段（Pass 2）
        //=========================================================================
        // 现在绑定回默认的帧缓冲区并使用附加的帧缓冲区颜色纹理绘制四边形平面
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); // 禁用深度测试，因此深度测试不会丢弃屏幕空间四边形。
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // 设置清空屏幕后填充的颜色（白色）
        glClear(GL_COLOR_BUFFER_BIT);// 清空屏幕的 颜色缓冲

        screenShader.use();
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec); // 使用颜色附件纹理作为四边形平面的纹理
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
//        GORK::RenderQuad();
        
        

        glfwSwapBuffers(window); //交换颜色缓冲，用来绘制，并且将会作为输出显示在屏幕上。
        glfwPollEvents(); //检查有没有触发什么事件
    }
    // -------------------------------


    // 终止，释放/删除之前的分配的所有资源
    // -------------------------------
    glfwTerminate(); //（Terminate终止） 释放/删除之前的分配的所有资源
    return 0;
}


