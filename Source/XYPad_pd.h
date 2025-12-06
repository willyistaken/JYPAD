#pragma once

#include "m_pd.h"
#include "XYPad.h"

//==============================================================================
/**
 * Pure Data 外部物件包裝
 * 將 XYPad 包裝成 Pure Data 物件，可以在 PlugData 中直接使用
 */
class XYPad_pd
{
public:
    // Pure Data 物件設置
    static void* setup(t_class* c);
    static void* new_instance(t_symbol* s, int argc, t_atom* argv);
    static void destroy(XYPad_pd* x);
    
    // Pure Data 訊息處理
    static void bang(XYPad_pd* x);
    static void float_method(XYPad_pd* x, t_floatarg f);
    static void symbol_method(XYPad_pd* x, t_symbol* s);
    static void list_method(XYPad_pd* x, t_symbol* s, int argc, t_atom* argv);
    
    // 自訂訊息
    static void add_ball(XYPad_pd* x, t_symbol* s, int argc, t_atom* argv);
    static void remove_ball(XYPad_pd* x, t_symbol* s, int argc, t_atom* argv);
    static void set_position(XYPad_pd* x, t_symbol* s, int argc, t_atom* argv);
    static void get_position(XYPad_pd* x, t_symbol* s, int argc, t_atom* argv);
    
    // 輸出回調
    void onBallMovedCallback(int ballId, float x, float y);
    
private:
    t_object x_obj;      // Pure Data 物件
    t_outlet* outlet_list;     // 輸出 outlet（輸出 list: 球編號 x y）
    XYPad xyPad;               // XYPad 實例
    
    XYPad_pd();
    ~XYPad_pd();
};

