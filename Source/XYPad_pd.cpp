#include "XYPad_pd.h"
#include <cstring>

//==============================================================================
// Pure Data 類別定義
static t_class* xypad_class = nullptr;

//==============================================================================
XYPad_pd::XYPad_pd()
    : outlet_list(nullptr)
{
    // 設定輸出回調
    xyPad.onBallMoved = [this](int ballId, float x, float y) {
        onBallMovedCallback(ballId, x, y);
    };
}

XYPad_pd::~XYPad_pd()
{
}

//==============================================================================
void* XYPad_pd::setup(t_class* c)
{
    xypad_class = c;
    
    // 註冊標準訊息
    class_addbang(c, (t_method)bang);
    class_addfloat(c, (t_method)float_method);
    class_addsymbol(c, (t_method)symbol_method);
    class_addlist(c, (t_method)list_method);
    
    // 註冊自訂訊息
    class_addmethod(c, (t_method)add_ball, gensym("add"), A_GIMME, 0);
    class_addmethod(c, (t_method)remove_ball, gensym("remove"), A_GIMME, 0);
    class_addmethod(c, (t_method)set_position, gensym("set"), A_GIMME, 0);
    class_addmethod(c, (t_method)get_position, gensym("get"), A_GIMME, 0);
    
    return nullptr;
}

void* XYPad_pd::new_instance(t_symbol* s, int argc, t_atom* argv)
{
    XYPad_pd* x = new XYPad_pd();
    
    // 創建 outlet
    x->outlet_list = outlet_new(&x->x_obj, &s_list);
    
    // 處理參數（可選：初始球數量）
    if (argc > 0 && argv[0].a_type == A_FLOAT)
    {
        int numBalls = (int)atom_getfloat(&argv[0]);
        for (int i = 1; i <= numBalls; ++i)
        {
            x->xyPad.addBall(i, 0.0f, 0.0f);
        }
    }
    else
    {
        // 預設添加一個球
        x->xyPad.addBall(1, 0.0f, 0.0f);
    }
    
    return (void*)x;
}

void XYPad_pd::destroy(XYPad_pd* x)
{
    if (x->outlet_list)
        outlet_free(x->outlet_list);
    delete x;
}

//==============================================================================
void XYPad_pd::bang(XYPad_pd* x)
{
    // 輸出所有球的位置
    const auto& balls = x->xyPad.getAllBalls();
    for (const auto& ball : balls)
    {
        t_atom out[3];
        SETFLOAT(&out[0], (float)ball.id);
        SETFLOAT(&out[1], ball.x);
        SETFLOAT(&out[2], ball.y);
        outlet_list(x->outlet_list, nullptr, 3, out);
    }
}

void XYPad_pd::float_method(XYPad_pd* x, t_floatarg f)
{
    // 如果收到 float，視為球編號，輸出該球的位置
    int ballId = (int)f;
    Ball* ball = x->xyPad.getBall(ballId);
    if (ball != nullptr)
    {
        t_atom out[3];
        SETFLOAT(&out[0], (float)ball->id);
        SETFLOAT(&out[1], ball->x);
        SETFLOAT(&out[2], ball->y);
        outlet_list(x->outlet_list, nullptr, 3, out);
    }
}

void XYPad_pd::symbol_method(XYPad_pd* x, t_symbol* s)
{
    // 可以處理符號訊息
}

void XYPad_pd::list_method(XYPad_pd* x, t_symbol* s, int argc, t_atom* argv)
{
    // list 格式: [球編號 x y] - 設定球的位置
    if (argc >= 3)
    {
        int ballId = (int)atom_getfloat(&argv[0]);
        float x = atom_getfloat(&argv[1]);
        float y = atom_getfloat(&argv[2]);
        x->xyPad.setBallPosition(ballId, x, y);
    }
}

//==============================================================================
void XYPad_pd::add_ball(XYPad_pd* x, t_symbol* s, int argc, t_atom* argv)
{
    // add 訊息格式: [add 球編號 x y] 或 [add 球編號]
    if (argc >= 1)
    {
        int ballId = (int)atom_getfloat(&argv[0]);
        float xPos = 0.0f;
        float yPos = 0.0f;
        
        if (argc >= 3)
        {
            xPos = atom_getfloat(&argv[1]);
            yPos = atom_getfloat(&argv[2]);
        }
        
        x->xyPad.addBall(ballId, xPos, yPos);
    }
}

void XYPad_pd::remove_ball(XYPad_pd* x, t_symbol* s, int argc, t_atom* argv)
{
    // remove 訊息格式: [remove 球編號]
    if (argc >= 1)
    {
        int ballId = (int)atom_getfloat(&argv[0]);
        x->xyPad.removeBall(ballId);
    }
}

void XYPad_pd::set_position(XYPad_pd* x, t_symbol* s, int argc, t_atom* argv)
{
    // set 訊息格式: [set 球編號 x y]
    if (argc >= 3)
    {
        int ballId = (int)atom_getfloat(&argv[0]);
        float xPos = atom_getfloat(&argv[1]);
        float yPos = atom_getfloat(&argv[2]);
        x->xyPad.setBallPosition(ballId, xPos, yPos);
    }
}

void XYPad_pd::get_position(XYPad_pd* x, t_symbol* s, int argc, t_atom* argv)
{
    // get 訊息格式: [get 球編號]
    if (argc >= 1)
    {
        int ballId = (int)atom_getfloat(&argv[0]);
        Ball* ball = x->xyPad.getBall(ballId);
        if (ball != nullptr)
        {
            t_atom out[3];
            SETFLOAT(&out[0], (float)ball->id);
            SETFLOAT(&out[1], ball->x);
            SETFLOAT(&out[2], ball->y);
            outlet_list(x->outlet_list, nullptr, 3, out);
        }
    }
}

//==============================================================================
void XYPad_pd::onBallMovedCallback(int ballId, float x, float y)
{
    // 當球移動時，輸出 list: 球編號 x y
    t_atom out[3];
    SETFLOAT(&out[0], (float)ballId);
    SETFLOAT(&out[1], x);
    SETFLOAT(&out[2], y);
    outlet_list(outlet_list, nullptr, 3, out);
}

//==============================================================================
// Pure Data 外部物件入口點
extern "C" {
    void xypad_setup(void)
    {
        t_class* c = class_new(gensym("xypad"),
                               (t_newmethod)XYPad_pd::new_instance,
                               (t_method)XYPad_pd::destroy,
                               sizeof(XYPad_pd),
                               CLASS_DEFAULT,
                               A_GIMME, 0);
        
        XYPad_pd::setup(c);
        class_sethelpsymbol(c, gensym("xypad"));
    }
}

