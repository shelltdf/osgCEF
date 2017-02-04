
#include <iostream>
#include <thread>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgDB/fstream>
#include <osgViewer/Viewer>
#include <osg/TexMat>

#include <osgCEF/Browser>


int main( int argc, char **argv )
{
    //viewer
    osgViewer::Viewer viewer;
    osg::Group* root = new osg::Group();
    viewer.setSceneData(root);
    //viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
    viewer.setUpViewInWindow(100, 100, 1280, 720);

    //
    CEF::init();
    Browser* b = new Browser();
    b->load("www.bing.com");
    //b->load("https://get.webgl.org/");
    //b->load("html5test.com");

    //
    osg::Geometry* geom = osg::createTexturedQuadGeometry(osg::Vec3(-1, -1, 0), osg::Vec3(2, 0, 0), osg::Vec3(0, 2, 0));
    osg::Geode* geode = new osg::Geode();
    geode->addDrawable(geom);
    b->addChild(geode);

    //no lighting
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, false);
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, new osg::TexMat(osg::Matrix::scale(1, -1, 1)));


    //hud
    if (true)
    {
        osg::Camera* camera = new osg::Camera();

        camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
        camera->setProjectionMatrixAsOrtho(-1, 1, -1, 1, 0, 1);
        camera->setViewMatrixAsLookAt(osg::Vec3(0, 0, 0.999999), osg::Vec3(0, 0, 0), osg::Vec3(0, 1, 0));
        //camera->getOrCreateStateSet()->setMode(GL_LIGHTING, false);

        camera->addChild(b);
        root->addChild(camera);
    }
    else
    {
        root->addChild(b);
    }

    //force resize event
    viewer.frame();
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(viewer.getCamera()->getGraphicsContext());
    if (gw)
    {
        // Send window size event for initializing
        int x, y, w, h; 
        gw->getWindowRectangle(x, y, w, h);
        viewer.getEventQueue()->windowResize(x, y, w, h);
    }

    //run
    //viewer.run();
    while (!viewer.done())
    {
        CEF::update();
        viewer.frame();
    }

    //shutdwon
    CEF::shutdown();

    return 0;
}
