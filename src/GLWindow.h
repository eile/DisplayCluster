#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include "TextureFactory.h"
#include "DynamicTextureFactory.h"
#include <QGLWidget>

class GLWindow : public QGLWidget
{

    public:

        GLWindow();
        ~GLWindow();

        TextureFactory & getTextureFactory();
        DynamicTextureFactory & getDynamicTextureFactory();

        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);
        void setView(int width, int height);

        static void drawRectangle(double x, double y, double w, double h);

    private:

        TextureFactory textureFactory_;
        DynamicTextureFactory dynamicTextureFactory_;
};

#endif
