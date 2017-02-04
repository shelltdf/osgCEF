
#include <osgCEF/Browser>

#include <osgCEF/CEF>

void configureShaders(osg::StateSet* stateSet)
{
    const std::string vertexSource =
        "varying vec4 TextureCoord0;\n"
        "void main()\n"
        "{\n"
        "   TextureCoord0 = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
        "   gl_Position = ftransform();\n"
        "}\n"
        "\n";
    osg::Shader* vShader = new osg::Shader(osg::Shader::VERTEX, vertexSource);

    const std::string fragmentSource =
        "varying vec4 TextureCoord0;\n"
        "uniform sampler2D tex_color;\n"
        "void main()\n"
        "{\n"
        "   vec4 color = texture2D(tex_color,TextureCoord0.st);\n"
        "   gl_FragColor = vec4(color.b,color.g,color.r,color.a);\n"
        "}\n"
        "\n";
    osg::Shader* fShader = new osg::Shader(osg::Shader::FRAGMENT, fragmentSource);

    osg::Program* program = new osg::Program;
    program->addShader(vShader);
    program->addShader(fShader);
    stateSet->setAttributeAndModes(program);
}

class RenderHandler 
    : public CefRenderHandler 
{
public:
    RenderHandler()
        : have_new(false)
        , wip(false)
    {
        image = new osg::Image();
        image->setDataVariance(osg::Object::DYNAMIC);

        resize(512, 512);
    }

    void resize(int w, int h)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);

        width = w;
        height = h;
    }

    bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);

        rect = CefRect(0, 0, width, height);
        return true;
    }

    void OnPaint(CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList &dirtyRects,
        const void* buffer,
        int _width,
        int _height) override
    {
        wip = true;
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);
        
        //glBindTexture(GL_TEXTURE_2D, texture);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (unsigned char*)buffer);

        //resize
        if ((image->s() != width) || (image->t() != height))
        {
            //printf("r");
            image->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
        }

        //copy
        int size = std::min<int>(width * height * 4, _width * _height * 4);
        unsigned char* data = image->data();
        memcpy((void*)data, buffer, size);

        //image->setImage(width, height, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE
        //    , (unsigned char*)buffer, osg::Image::AllocationMode::NO_DELETE);

        have_new = true;
        wip = false;
    }

    bool getImage(osg::ref_ptr<osg::Image>& img)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);
        
        if (have_new)
        {
            //OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);

            image.swap(img);
            have_new = false;
            return true;
        }

        return false;
    }

    
    bool wip;

private:
    OpenThreads::Mutex mutex;
    bool have_new;

    int width;
    int height;

    osg::ref_ptr<osg::Image> image;

    IMPLEMENT_REFCOUNTING(RenderHandler);
};


class Client 
    : public CefClient {
public:
    Client(RenderHandler* rh)
    {
        renderHandler = rh;
    }

    virtual CefRefPtr<CefRenderHandler> GetRenderHandler()
    {
        return renderHandler;
    };

private:
    CefRefPtr<CefRenderHandler> renderHandler;

    IMPLEMENT_REFCOUNTING(Client);
};


class BrowserEventHandler
    :public osgGA::GUIEventHandler
{
public:
    BrowserEventHandler()
    :width(0), height(0){}
    virtual ~BrowserEventHandler() {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* obj, osg::NodeVisitor* nv) override
    {
        if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
        {
            browser->update();
        }
        else if (ea.getEventType() == osgGA::GUIEventAdapter::RESIZE)
        {
            if ((width != ea.getWindowWidth()) || (height != ea.getWindowHeight()))
            {
                browser->resize(ea.getWindowWidth(), ea.getWindowHeight());

                width = ea.getWindowWidth();
                height = ea.getWindowHeight();
            }
        }
        else if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE)
        {
            browser->mouseMove(ea.getX(), ea.getWindowHeight() - ea.getY());
        }
        else if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
        {
            int b = 0;
            if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) b = 1;
            if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON) b = 2;
            if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) b = 3;

            browser->mouseClick(b, true);
        }
        else if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
        {
            int b = 0;
            if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) b = 1;
            if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON) b = 2;
            if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) b = 3;

            browser->mouseClick(b, false);
        }
        else if (ea.getEventType() == osgGA::GUIEventAdapter::SCROLL)
        {
            if (ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP)
            {
                browser->MouseWheel(true);
            }
            else if (ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_DOWN)
            {
                browser->MouseWheel(false);
            }
        }

        return osgGA::GUIEventHandler::handle(ea, aa, obj, nv);
    }

    int width;
    int height;

    Browser* browser;
};



Browser::Browser()
{
    //texture
    texture = new osg::Texture2D();
    texture->setResizeNonPowerOfTwoHint(false);
    texture->setUseHardwareMipMapGeneration(false);
    texture->setTextureSize(512, 512);
    texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
    texture->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
    texture->setDataVariance(osg::Object::DYNAMIC);

    //image
    image = new osg::Image();
    image->setDataVariance(osg::Object::DYNAMIC);
    texture->setImage(image);

    //stateset
    this->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture);
    configureShaders(this->getOrCreateStateSet());

    //cef window info
    CefWindowInfo windowInfo;
    //windowInfo.SetAsOffScreen(nullptr);
    //windowInfo.SetTransparentPainting(true);
    windowInfo.SetAsWindowless(0, true);

    // render handler
    RenderHandler* rh = new RenderHandler(/*texture, image, &mutex*/);
    renderHandler = rh;

    //client and browser
    CefRefPtr<Client> client = new Client(rh);
    CefBrowserSettings settings;
    //settings.windowless_frame_rate = 60;
    browser = CefBrowserHost::CreateBrowserSync(windowInfo, client.get(), "", settings, nullptr);
    //browser->GetHost()->SetWindowlessFrameRate(120);

    //event handler
    BrowserEventHandler* eh = new BrowserEventHandler();
    eh->browser = this;
    this->setEventCallback(eh);
}

Browser::~Browser()
{
    //browser->GetHost()->CloseBrowser(true);

    browser = 0;
    renderHandler = 0;
}

void Browser::load(const char* url)
{
    browser->GetMainFrame()->LoadURL(url);
}

void Browser::resize(int w, int h)
{
    RenderHandler* rh = (RenderHandler*)renderHandler;
    rh->resize(w, h);

    browser->GetHost()->WasResized();
}

void Browser::update()
{
    RenderHandler* rh = (RenderHandler*)renderHandler;
    
    //if (rh->mutex.lock() == 0)
    if ((!rh->wip))
    {
        //printf("-");
        //CefDoMessageLoopWork();

        //rh->mutex.unlock();


        //browser->GetHost()->Invalidate(PET_VIEW);

        //RenderHandler* rh = (RenderHandler*)renderHandler;

        bool b = rh->getImage(image);
        if (b) texture->dirtyTextureObject();
    }
}

void Browser::mouseMove(int x, int y)
{
    mouseX = x;
    mouseY = y;

    CefMouseEvent event;
    event.x = x;
    event.y = y;

    browser->GetHost()->SendMouseMoveEvent(event, false);
}
void Browser::mouseClick(int mouse_btn, int push_down)
{
    CefMouseEvent event;
    event.x = mouseX;
    event.y = mouseY;

    CefBrowserHost::MouseButtonType btnType = MBT_LEFT;
    if (mouse_btn == 2) btnType = MBT_MIDDLE;
    if (mouse_btn == 3) btnType = MBT_RIGHT;

    browser->GetHost()->SendMouseClickEvent(event, btnType, !push_down, 1);

}
void Browser::MouseWheel(int up)
{ 
    CefMouseEvent event;
    event.x = mouseX;
    event.y = mouseY;

    int x = 0;
    int y = 0;

    if (up)  y = 30;
    else y = -30;

    browser->GetHost()->SendMouseWheelEvent(event, x, y);
}
void Browser::keyPress(int key)
{
}


void Browser::executeJS(const char* command)
{
    CefRefPtr<CefFrame> frame = browser->GetMainFrame();
    frame->ExecuteJavaScript(command, frame->GetURL(), 0);
}

