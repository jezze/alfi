#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if defined NVG_GL_GLEW
#include <GL/glew.h>
#endif

#if defined NVG_GL_VERSION_GLES2
#include <GLES2/gl2.h>
#endif

#if defined NVG_GL_VERSION_GLES3
#include <GLES3/gl3.h>
#endif

#include <GLFW/glfw3.h>
#include "list.h"
#include "style.h"
#include "url.h"
#include "resource.h"
#include "view.h"
#include "attributes.h"
#include "widgets.h"
#include "entity.h"
#include "animation.h"
#include "parser.h"
#include "pool.h"
#include "render.h"
#include "history.h"

static struct view view;
static struct parser parser;
static double mouse_x;
static double mouse_y;
static GLFWcursor *cursor_arrow;
static GLFWcursor *cursor_ibeam;
static GLFWcursor *cursor_hand;
static struct widget *widget_root;
static struct widget *widget_main;
static struct widget *widget_hover;
static struct widget *widget_focus;
static unsigned int updatetitle;

static struct widget *parser_create(unsigned int type, char *id, char *in)
{

    struct widget *widget = pool_widget_create();

    entity_create(widget, type, id, in);

    return widget;

}

static struct widget *parser_destroy(struct widget *widget)
{

    entity_destroy(widget);

    return pool_widget_destroy(widget);

}

static void parser_clear(struct widget *widget)
{

    struct widget *child = 0;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        parser_clear(child);

        child = parser_destroy(child);

    }

}

static void parser_fail(void)
{

    printf("Parsing failed on line %u\n", parser.expr.line + 1);
    exit(EXIT_FAILURE);

}

static unsigned int buildkeyvalue(void *buffer, unsigned int count, unsigned int offset, char *key, char *value)
{

    if (offset)
        return snprintf(buffer, count, "&%s=%s", key, value);
    else
        return snprintf(buffer, count, "%s=%s", key, value);

}

static unsigned int builddata(char *buffer, unsigned int count)
{

    struct widget *widget = 0;
    unsigned int offset = 0;

    while ((widget = pool_widget_next(widget)))
    {

        if (!strlen(widget->header.id.name))
            continue;

        switch (widget->header.type)
        {

        case WIDGET_TYPE_FIELD:
            offset += buildkeyvalue(buffer + offset, count - offset, offset, widget->header.id.name, widget->payload.field.data.content);

            break;

        case WIDGET_TYPE_SELECT:
            offset += buildkeyvalue(buffer + offset, count - offset, offset, widget->header.id.name, widget->payload.select.data.content);

            break;

        case WIDGET_TYPE_TOGGLE:
            if (widget->payload.toggle.mode.type == ATTRIBUTE_MODE_ON)
                offset += buildkeyvalue(buffer + offset, count - offset, offset, widget->header.id.name, widget->payload.select.data.content);

            break;

        }

    }

    return offset;

}

static struct widget *findtouchingwidget(struct widget *widget, float x, float y)
{

    struct widget *child = 0;

    while ((child = pool_widget_nextchild(child, widget)))
    {

        if (style_box_istouching(&child->frame.bounds, x, y))
            return findtouchingwidget(child, x, y);

    }

    return widget;

}

static struct widget *prevflag(struct widget *widget, unsigned int flag)
{

    while ((widget = pool_widget_prev(widget)))
    {

        if (entity_checkflag(widget, flag))
            return widget;

    }

    return 0;

}

static struct widget *nextflag(struct widget *widget, unsigned int flag)
{

    while ((widget = pool_widget_next(widget)))
    {

        if (entity_checkflag(widget, flag))
            return widget;

    }

    return 0;

}

static void setcursor(struct GLFWwindow *window, unsigned int type)
{

    switch (type)
    {

    case ANIMATION_CURSOR_HAND:
        glfwSetCursor(window, cursor_hand);

        break;

    case ANIMATION_CURSOR_IBEAM:
        glfwSetCursor(window, cursor_ibeam);

        break;

    case ANIMATION_CURSOR_ARROW:
        glfwSetCursor(window, cursor_arrow);

        break;

    default:
        glfwSetCursor(window, cursor_arrow);

        break;

    }

}

static void setfocus(struct widget *widget)
{

    if (widget != widget_focus)
    {

        if (widget_focus)
            entity_setstate(widget_focus, WIDGET_STATE_UNFOCUS);

        widget_focus = widget;

        if (widget_focus)
            entity_setstate(widget_focus, WIDGET_STATE_FOCUS);

    }

}

static void sethover(struct widget *widget)
{

    if (widget != widget_hover)
    {

        if (widget_hover)
            entity_setstate(widget_hover, WIDGET_STATE_UNHOVER);

        widget_hover = widget;

        if (widget_hover)
            entity_setstate(widget_hover, WIDGET_STATE_HOVER);

    }

}

static void loadresources_code(struct widget_payload_code *payload)
{

    struct resource resource;
    struct urlinfo info;
    char *data;

    if (!strlen(payload->link.url))
        return;

    url_merge(&info, history_geturl(0), payload->link.url);
    attribute_link_create(&payload->link, info.url, payload->link.mime);
    resource_init(&resource, payload->link.url);
    resource_load(&resource, 0, 0);

    if (resource.count)
    {

        data = resource.data;
        data[resource.count - 1] = '\0';

        attribute_label_create(&payload->label, data);

    }

    resource_unload(&resource);
    resource_destroy(&resource);

}

static void loadresources_image(struct widget_payload_image *payload)
{

    struct urlinfo info;

    if (!strlen(payload->link.url))
        return;

    url_merge(&info, history_geturl(0), payload->link.url);
    attribute_link_create(&payload->link, info.url, payload->link.mime);
    render_loadimage(payload->link.url);

}

static void loadresources_text(struct widget_payload_text *payload)
{

    struct resource resource;
    struct urlinfo info;
    char *data;

    if (!strlen(payload->link.url))
        return;

    url_merge(&info, history_geturl(0), payload->link.url);
    attribute_link_create(&payload->link, info.url, payload->link.mime);
    resource_init(&resource, payload->link.url);
    resource_load(&resource, 0, 0);

    if (resource.count)
    {

        data = resource.data;
        data[resource.count - 1] = '\0';

        attribute_label_create(&payload->label, data);

    }

    resource_unload(&resource);
    resource_destroy(&resource);

}

static void loadresources(void)
{

    struct widget *widget = 0;

    while ((widget = pool_widget_next(widget)))
    {

        switch (widget->header.type)
        {

        case WIDGET_TYPE_CODE:
            loadresources_code(&widget->payload.code);

            break;

        case WIDGET_TYPE_IMAGE:
            loadresources_image(&widget->payload.image);

            break;

        case WIDGET_TYPE_TEXT:
            loadresources_text(&widget->payload.text);

            break;

        }

    }

}

static void urlself(char *url, unsigned int count, void *data)
{

    struct resource temp;

    resource_init(&temp, url);
    resource_load(&temp, count, data);

    if (temp.count)
    {

        struct urlinfo *info = history_get(0);

        if (info)
            url_set(info, temp.urlinfo.url);

        parser_parse(&parser, "main", temp.count, temp.data);
        loadresources();
        animation_step(widget_root, view.scrollx, view.scrolly, view.scrollw, &view, 1.0);

        updatetitle = 1;

    }

    resource_unload(&temp);
    resource_destroy(&temp);

}

static void urlblank(char *url, unsigned int count, void *data)
{

    parser_clear(widget_main);
    view_reset(&view);
    view_adjust(&view, widget_root->frame.bounds.w, widget_root->frame.bounds.h);
    urlself(url, count, data);

}

static void loadself(char *url, unsigned int count, void *data)
{

    struct urlinfo info;

    url_merge(&info, history_geturl(0), url);
    urlself(info.url, count, data);

}

static void loadblank(char *url, unsigned int count, void *data)
{

    struct urlinfo *info = history_push();

    url_merge(info, history_geturl(1), url);
    urlblank(info->url, count, data);

}

static void create(char *title)
{

    widget_root = parser_create(WIDGET_TYPE_WINDOW, "window", "");
    widget_main = parser_create(WIDGET_TYPE_TABLE, "main", "window");

    attribute_label_create(&widget_root->payload.window.label, title);

}

static void onerror(int error, const char *desc)
{

    printf("GLFW error %d: %s\n", error, desc);

}

static void onwindowsize(GLFWwindow *window, int width, int height)
{

    view_init(&view, width, height);

}

static void onframebuffersize(GLFWwindow *window, int width, int height)
{

    glViewport(0, 0, width, height);

}

static void oninput_field(struct widget_payload_field *payload, int key, int scancode, int action, int mods)
{

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {

        switch (key)
        {

        case GLFW_KEY_LEFT:
            if (payload->data.offset > 0)
                payload->data.offset--;

            break;

        case GLFW_KEY_RIGHT:
            if (payload->data.offset < strlen(payload->data.content))
                payload->data.offset++;

            break;

        case GLFW_KEY_ENTER:
            if (payload->data.offset < payload->data.total - 1)
            {

                payload->data.content[payload->data.offset] = '\n';
                payload->data.offset++;
                payload->data.content[payload->data.offset] = '\0';

            }

            break;

        case GLFW_KEY_BACKSPACE:
            if (payload->data.offset > 0)
            {

                payload->data.content[payload->data.offset] = '\0';
                payload->data.offset--;
                payload->data.content[payload->data.offset] = '\0';

            }

            break;

        }

    }

}

static void oninput_select(struct widget_payload_select *payload, int key, int scancode, int action, int mods)
{

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {

        switch (key)
        {

        case GLFW_KEY_UP:
            break;

        case GLFW_KEY_DOWN:
            break;

        case GLFW_KEY_ENTER:
            break;

        }

    }

}

static void oninput_toggle(struct widget_payload_toggle *payload, int key, int scancode, int action, int mods)
{

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {

        switch (key)
        {

        case GLFW_KEY_SPACE:
            switch (payload->mode.type)
            {

            case ATTRIBUTE_MODE_OFF:
                payload->mode.type = ATTRIBUTE_MODE_ON;

                break;

            case ATTRIBUTE_MODE_ON:
                payload->mode.type = ATTRIBUTE_MODE_OFF;

                break;

            }

            break;

        }

    }

}

static void onkey(GLFWwindow *window, int key, int scancode, int action, int mods)
{

    if (action == GLFW_PRESS)
    {

        switch (key)
        {

        case GLFW_KEY_B:
            if (mods & GLFW_MOD_CONTROL)
                loadblank("navi://bookmarks", 0, 0);

            break;

        case GLFW_KEY_D:
            if (mods & GLFW_MOD_CONTROL)
                loadblank("file://", 0, 0);

            break;

        case GLFW_KEY_H:
            if (mods & GLFW_MOD_CONTROL)
                loadblank("navi://history", 0, 0);

            break;

        case GLFW_KEY_L:
            if (mods & GLFW_MOD_CONTROL)
                loadblank("navi://blank", 0, 0);

            break;

        case GLFW_KEY_M:
            if (mods & GLFW_MOD_CONTROL)
                animation_settheme(ANIMATION_THEME_DARK);

            break;

        case GLFW_KEY_N:
            if (mods & GLFW_MOD_CONTROL)
                animation_settheme(ANIMATION_THEME_LIGHT);

            break;

        case GLFW_KEY_Q:
            if (mods & GLFW_MOD_CONTROL)
                glfwSetWindowShouldClose(window, GL_TRUE);

            break;

        case GLFW_KEY_R:
            if (mods & GLFW_MOD_CONTROL)
                urlblank(history_geturl(0), 0, 0);

            break;

        }

    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {

        switch (key)
        {

        case GLFW_KEY_TAB:
            if (mods & GLFW_MOD_SHIFT)
            {

                struct widget *widget = prevflag(widget_focus, ENTITY_FLAG_FOCUSABLE);

                if (!widget)
                    widget = prevflag(0, ENTITY_FLAG_FOCUSABLE);

                setfocus(widget);

            }

            else
            {

                struct widget *widget = nextflag(widget_focus, ENTITY_FLAG_FOCUSABLE);

                if (!widget)
                    widget = nextflag(0, ENTITY_FLAG_FOCUSABLE);

                setfocus(widget);

            }

            break;

        }

    }

    if (widget_focus && entity_checkflag(widget_focus, ENTITY_FLAG_FOCUSABLE))
    {

        switch (widget_focus->header.type)
        {

        case WIDGET_TYPE_FIELD:
            oninput_field(&widget_focus->payload.field, key, scancode, action, mods);

            break;

        case WIDGET_TYPE_SELECT:
            oninput_select(&widget_focus->payload.select, key, scancode, action, mods);

            break;

        case WIDGET_TYPE_TOGGLE:
            oninput_toggle(&widget_focus->payload.toggle, key, scancode, action, mods);

            break;

        }

    }

    else
    {

        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {

            switch (key)
            {

            case GLFW_KEY_UP:
                view_scroll(&view, 0, 1);

                break;

            case GLFW_KEY_DOWN:
                view_scroll(&view, 0, -1);

                break;

            case GLFW_KEY_LEFT:
                view_scroll(&view, 1, 0);

                break;

            case GLFW_KEY_RIGHT:
                view_scroll(&view, -1, 0);

                break;

            case GLFW_KEY_PAGE_UP:
                view_scroll(&view, 0, 16);

                break;

            case GLFW_KEY_PAGE_DOWN:
                view_scroll(&view, 0, -16);

                break;

            case GLFW_KEY_HOME:
                view_scroll(&view, 16, 0);

                break;

            case GLFW_KEY_END:
                view_scroll(&view, -16, 0);

                break;

            }

        }

    }

}

static void onclick_anchor(struct widget *widget, float x, float y)
{

    struct widget_payload_anchor *payload = &widget->payload.anchor;
    struct frame *frame = &widget->frame;

    if (!style_box_istouching(&frame->styles[0].box, x, y))
        return;

    if (!strlen(payload->link.url))
        return;

    switch (payload->target.type)
    {

    case ATTRIBUTE_TARGET_SELF:
        loadself(payload->link.url, 0, 0);

        break;

    case ATTRIBUTE_TARGET_BLANK:
        loadblank(payload->link.url, 0, 0);

        break;

    }

}

static void onclick_button(struct widget *widget, float x, float y)
{

    struct widget_payload_button *payload = &widget->payload.button;
    struct frame *frame = &widget->frame;
    char data[RESOURCE_PAGESIZE];

    if (!style_box_istouching(&frame->styles[0].box, x, y))
        return;

    setfocus(widget);

    if (!strlen(payload->link.url))
        return;

    switch (payload->target.type)
    {

    case ATTRIBUTE_TARGET_SELF:
        loadself(payload->link.url, builddata(data, RESOURCE_PAGESIZE), data);

        break;

    case ATTRIBUTE_TARGET_BLANK:
        loadblank(payload->link.url, builddata(data, RESOURCE_PAGESIZE), data);

        break;

    }

}

static void onclick_choice(struct widget *widget, float x, float y)
{

    struct widget *parent = pool_widget_find(widget->header.in.name);
    struct widget_payload_choice *payload = &widget->payload.choice;

    switch (parent->header.type)
    {

    case WIDGET_TYPE_SELECT:
        attribute_data_create(&parent->payload.select.data, payload->label.content);

        break;

    }

}

static void onclick_field(struct widget *widget, float x, float y)
{

    struct frame *frame = &widget->frame;

    if (!style_box_istouching(&frame->styles[0].box, x, y))
        return;

    setfocus(widget);

}

static void onclick_select(struct widget *widget, float x, float y)
{

    struct frame *frame = &widget->frame;

    if (!style_box_istouching(&frame->styles[0].box, x, y))
        return;

    setfocus(widget);

}

static void onclick_toggle(struct widget *widget, float x, float y)
{

    struct widget_payload_toggle *payload = &widget->payload.toggle;
    struct frame *frame = &widget->frame;

    if (!style_box_istouching(&frame->styles[0].box, x, y))
        return;

    setfocus(widget);

    switch (payload->mode.type)
    {

    case ATTRIBUTE_MODE_OFF:
        payload->mode.type = ATTRIBUTE_MODE_ON;

        break;

    case ATTRIBUTE_MODE_ON:
        payload->mode.type = ATTRIBUTE_MODE_OFF;

        break;

    }

}

static void onbutton(GLFWwindow *window, int button, int action, int mods)
{

    if (action == GLFW_PRESS)
    {

        switch (button)
        {

        case GLFW_MOUSE_BUTTON_LEFT:
            setfocus(0);

            if (widget_hover)
            {

                switch (widget_hover->header.type)
                {

                case WIDGET_TYPE_ANCHOR:
                    onclick_anchor(widget_hover, mouse_x, mouse_y);

                    break;

                case WIDGET_TYPE_BUTTON:
                    onclick_button(widget_hover, mouse_x, mouse_y);

                    break;

                case WIDGET_TYPE_CHOICE:
                    onclick_choice(widget_hover, mouse_x, mouse_y);

                    break;

                case WIDGET_TYPE_FIELD:
                    onclick_field(widget_hover, mouse_x, mouse_y);

                    break;

                case WIDGET_TYPE_SELECT:
                    onclick_select(widget_hover, mouse_x, mouse_y);

                    break;

                case WIDGET_TYPE_TOGGLE:
                    onclick_toggle(widget_hover, mouse_x, mouse_y);

                    break;

                }

            }

            break;

        case GLFW_MOUSE_BUTTON_MIDDLE:
            loadblank("navi://blank", 0, 0);

            break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            if (mods & GLFW_MOD_SHIFT)
            {

                if (widget_hover)
                {

                    parser_clear(widget_hover);

                    widget_hover = parser_destroy(widget_hover);

                    sethover(widget_hover);

                }

            }

            else
            {

                struct urlinfo *info = history_pop();

                if (info)
                    urlblank(info->url, 0, 0);

            }

            break;

        }

    }

}

static void oncursor(GLFWwindow *window, double x, double y)
{

    mouse_x = x;
    mouse_y = y;

}

static void onscroll(GLFWwindow *window, double x, double y)
{

    view_scroll(&view, x, y);

}

static void onchar_field(struct widget_payload_field *payload, unsigned int codepoint)
{

    if (payload->data.offset < payload->data.total - 1)
    {

        payload->data.content[payload->data.offset] = codepoint;
        payload->data.offset++;
        payload->data.content[payload->data.offset] = '\0';

    }

}

static void onchar(GLFWwindow *window, unsigned int codepoint)
{

    if (widget_focus && entity_checkflag(widget_focus, ENTITY_FLAG_FOCUSABLE))
    {

        switch (widget_focus->header.type)
        {

        case WIDGET_TYPE_FIELD:
            onchar_field(&widget_focus->payload.field, codepoint);

            break;

        }

    }

}

static void checkhover(GLFWwindow *window)
{

    struct widget *touch = findtouchingwidget(widget_root, mouse_x, mouse_y);

    sethover(touch);

    if (touch)
        setcursor(window, animation_getcursor(touch, mouse_x, mouse_y));

}

static void checktitle(GLFWwindow *window)
{

    if (updatetitle)
    {

        glfwSetWindowTitle(window, widget_root->payload.window.label.content);

        updatetitle = 0;

    }

}

static unsigned int checkanimating(void)
{

    struct widget *widget = 0;

    while ((widget = pool_widget_next(widget)))
    {

        struct frame *frame = &widget->frame;

        if (frame->animating)
            return 1;

    }

    return 0;

}

static void render(float u)
{

    render_reset(view.pagew, view.pageh);
    render_background(view.pagew, view.pageh, &widget_root->frame.styles[0].color);

    if (widget_root)
    {

        view_adjust(&view, widget_root->frame.bounds.w, widget_root->frame.bounds.h);
        animation_step(widget_root, view.scrollx, view.scrolly, view.scrollw, &view, u);
        animation_render(widget_root, &view);

    }

    render_flush();

}

static void run(GLFWwindow *window)
{

    double prevt = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {

        double t = glfwGetTime();
        double dt = t - prevt;
        unsigned int frames = 0;

        if (dt > 0.016)
        {

            frames++;
            prevt = t;

        }

        if (frames)
        {

            glfwPollEvents();
            checkhover(window);
            checktitle(window);
            render(0.5);
            glfwSwapBuffers(window);

            if (!checkanimating())
                glfwWaitEvents();

        }

    }

}

int main(int argc, char **argv)
{

    const GLFWvidmode *mode;
    GLFWmonitor *monitor;
    GLFWwindow *window;

    glfwInit();
    glfwSetErrorCallback(onerror);

    monitor = glfwGetPrimaryMonitor();
    mode = glfwGetVideoMode(monitor);
    window = glfwCreateWindow(mode->width, mode->height, "Navi 0.1", 0, 0);
    cursor_arrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    cursor_ibeam = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    cursor_hand = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

    glfwSetWindowSizeCallback(window, onwindowsize);
    glfwSetFramebufferSizeCallback(window, onframebuffersize);
    glfwSetKeyCallback(window, onkey);
    glfwSetMouseButtonCallback(window, onbutton);
    glfwSetCursorPosCallback(window, oncursor);
    glfwSetScrollCallback(window, onscroll);
    glfwSetCharCallback(window, onchar);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetTime(0);

#ifdef NVG_GL_GLEW
    glewInit();
#endif

    view_init(&view, mode->width, mode->height);
    render_create();
    parser_init(&parser, parser_fail, pool_widget_find, parser_create, parser_destroy, parser_clear);
    pool_setup();
    animation_setup();
    animation_setupfonts();
    animation_settheme(ANIMATION_THEME_LIGHT);
    create("Navi 0.1");
    render(1.0);
    loadblank("navi://blank", 0, 0);
    run(window);
    render_destroy();
    glfwTerminate();

    return 0;

}

