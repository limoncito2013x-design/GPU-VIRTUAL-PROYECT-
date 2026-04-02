#include <jni.h>
#include <android/native_window_jni.h>
#include <chrono>
#include <vector>
#include "rasterizer.h"
#include "math_utils.h"

static Rasterizer* g_rasterizer = nullptr;
static ANativeWindow* g_window = nullptr;
static int g_width = 0, g_height = 0;

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_gpuvirtual_MainActivity_nativeInit(JNIEnv* env, jobject thiz, jobject surface, jint width, jint height) {
    g_width = width;
    g_height = height;
    if (g_rasterizer) delete g_rasterizer;
    g_rasterizer = new Rasterizer(width, height);
    g_window = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_setBuffersGeometry(g_window, width, height, WINDOW_FORMAT_RGBA_8888);
}

JNIEXPORT void JNICALL
Java_com_example_gpuvirtual_MainActivity_nativeStep(JNIEnv* env, jobject thiz) {
    static auto lastTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;

    g_rasterizer->clear(0xFF002233);

    static float angle = 0;
    angle += deltaTime * 2.0f;

    Vec3 vertices[8] = {
        {-1,-1,-1}, { 1,-1,-1}, { 1,-1, 1}, {-1,-1, 1},
        {-1, 1,-1}, { 1, 1,-1}, { 1, 1, 1}, {-1, 1, 1}
    };

    float c = cos(angle), s = sin(angle);
    Mat4 rot;
    rot.m[0] = c; rot.m[2] = s;
    rot.m[5] = 1;
    rot.m[8] = -s; rot.m[10] = c;
    rot.m[15] = 1;

    Mat4 proj = Mat4::perspective(3.14159f/3.0f, (float)g_width/g_height, 0.1f, 10.0f);
    Vec3 eye(3, 2, 4);
    Vec3 target(0,0,0);
    Vec3 up(0,1,0);
    Mat4 view = Mat4::lookAt(eye, target, up);
    Mat4 mvp = proj * view * rot;

    std::vector<Triangle> triangles;
    int indices[12][3] = {
        {0,1,2}, {0,2,3},
        {4,6,5}, {4,7,6},
        {0,5,1}, {0,4,5},
        {1,6,2}, {1,5,6},
        {2,7,3}, {2,6,7},
        {3,4,0}, {3,7,4}
    };

    Vec3 screenVerts[8];
    for (int i = 0; i < 8; i++) {
        Vec3 v = mvp.transform(vertices[i]);
        float x = (v.x + 1.0f) * 0.5f * g_width;
        float y = (1.0f - (v.y + 1.0f) * 0.5f) * g_height;
        float z = v.z;
        screenVerts[i] = Vec3(x, y, z);
    }

    uint32_t colors[8] = {
        0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFF00,
        0xFFFF00FF, 0xFF00FFFF, 0xFFFFFFFF, 0xFF888888
    };

    for (int i = 0; i < 12; i++) {
        Triangle tri;
        tri.v0 = {screenVerts[indices[i][0]], colors[indices[i][0]]};
        tri.v1 = {screenVerts[indices[i][1]], colors[indices[i][1]]};
        tri.v2 = {screenVerts[indices[i][2]], colors[indices[i][2]]};
        triangles.push_back(tri);
    }

    g_rasterizer->drawTriangles(triangles);
    g_rasterizer->present(g_window);
    g_rasterizer->updateFPS(deltaTime);
}

JNIEXPORT jfloat JNICALL
Java_com_example_gpuvirtual_MainActivity_nativeGetFPS(JNIEnv* env, jobject thiz) {
    return g_rasterizer ? g_rasterizer->getFPS() : 0.0f;
}

} // extern "C"
