## On the usage of native surfaces in Krita

[[_TOC_]]

### Alien vs Native widgets

In Qt all widgets are split into two categories: "alien" and "native" [^1]. Native widgets have their own native surface, where they are rendered on. Alien widgets don't have any surface associated with them. Instead, they are rendered on the surface of the nearest native ancestor (see `QWidget::nativeParentWidget()`). Qt basically renders all alien widgets on a single texture and then just uploads it into a single native surface in one go. This feature significantly speeds up widget painting, resizing, and removes flicker.

Every native widget has a dedicated `QWindow` object that represents a native window. This `QWindow` may later be downcasted to a `QPlatformWindow` to get some platform-specific features of the window (e.g. color management on Wayland).

The rendering of all widgets is scheduled by `QWidgetRepaintManager`. The alien widgets are rendered with classes `QBackingStoreDefaultCompositor` and `QPlatformBackingStore`.

### Native widgets management API

* `widget->windowHandle() -> QWindow` actually distinguishes alien widgets from native ones. If a widget has a "window handle", i.e. has a dedicated QWindow, then it is a native widget. If `windowHandle()` returns null, then it is an alien widget.
* `widget->windowHandle()->handle() -> QPlatformWindow` gives you access to some platform specific APIs of native surfaces. This pointer can be used by to get access to platform-specific implementations, like `QWaylandWindow` or `QWindowsWindow`.
* `widget->nativeParentWidget() -> QWidget` returns the nearest ancestor that has a native surface. For alien widgets it basically returns a native widget on whose surface the alien widget is going to be rendered.
* `widget->window() -> QWidget` returns the **QWidget** wrapper for **the toplevel window the widget belongs to**. In case of Krita it will usually be either `KisMainWindow` or some `KoDialog`. Contrary to its name, this method has no relation to native vs alien widgets!


### Creation of native widgets

Normally, all widgets in the application are alien widgets and are rendered on the surface of the toplevel window using `QBackingStoreDefaultCompositor`. But the application can request a promotion of a widget into the native one by one of the two methods:

1) `widget->winId()` explicitly creates a native window for the widget. After the call to it, both, `widget->windowHandle()` and `widget->windowHandle()->handle()` will exist.
2) `widget->setAttribute(Qt::WA_NativeWindow, true);` marks the widget that it should be a native widget. If the widget has already been created, then it will be converted to a native one via a direct call to `winId()`. If the widget has not yet been created (has not been "shown" on screen at least once), then the native surface creation will be postponed till the next call to `QWidgetPrivate::create()`.

Krita's openGL canvas is marked with `Qt::WA_NativeWindow` in the constructor, hence it gets a native surface when the canvas is made visible for the first time.

> [!WARNING]
> Be careful, sometimes, if you set a `Qt::WA_NativeWindow` attribute to an already created (i.e. visible) widget and/or attach it to an already created (i.e. visible) parent, then Qt may fail to initialize it properly and will leave the widget in **half-initialized** state. In such a state the widget usually:
>
>   * has an associated `QWindow` (i.e. `widget->windowHandle() != nullptr` )
>   * but has **no platform window attached** (i.e. `widget->windowHandle()->handle() == nullptr`)
>
> This problem happens in the switch-case branch of `Qt::WA_NativeWindow` in `QWidget::setAttribute()`. It creates a toplevel info with `d->createTLSysExtra()` but skips calling `d->createWinId()` because `Qt::WA_WState_Created` is already set.
>
> The usual solution is to set the attribute **before** the widget is created (i.e. made visible) or just call `winId()` explicitly.

### Native parents and siblings

By default, if you make any widget native, Qt will automatically convert all its **parents** and **siblings** to native widgets as well. It is done to fully support proper handling of the z-index (i.e. ordering) of the widgets during rendering. Otherwise, it is impossible to render overlapping widgets "correctly from the theoretical point of view" [^2].

See this set of siblings for example:

```
MainWindow : QMainWindow
   ├── alienWidget1 : QWidget
   ├── nativeWidget2 : QWidget ## has its own surface, which is above the MainWindow's surface!
   └── alienWidget3 : QWidget ## drawn on MainWindow's surface, i.e. below nativeWidget2
```

According to the ordering of the widgets, `alienWidget3` should be painted on the top of `nativeWidget2`. But it is impossible, because the physical surface it is painted on is situated below the surface `nativeWidget2` is painted on!

The same problem happens with alien parents of the native widgets:

```
MainWindow : QMainWindow
   ├── alienWidget1 : QWidget
   ├── alienWidget2 : QWidget
   │    └── nativeWidget3 : QWidget ## has its own surface, which is above the MainWindow's surface!
   └── alienWidget4 : QWidget ## drawn on MainWindow's surface, i.e. below nativeWidget3
```

To establish total ordering of the widgets, Qt will just convert all the widgets of this set into native ones.

#### Relaxed widget ordering

Even though having correct ordering of all the widgets is good, having too many native surfaces can cause severe performance issues (e.g. FPS drops). That is why we disable that in Krita. Qt has two attributes to disable this automatic conversion of widgets:

* `QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true)` this **application-wide** attribute disables creation of the native siblings
* `widget->setAttribute(Qt::WA_DontCreateNativeAncestors, true)` this **per-widget** attribute disables automatic conversion of widget's parents into the native widgets. This attribute must be set **before** setting `Qt::WA_NativeWindow` or calling `winId()`.

Setting these switches breaks theoretical total rendering ordering of the widget hierarchy. I.e. all alien widgets will always be rendered below all the native widgets, whatever their real ordering should be. From the first glance, this sounds as a bad idea, but it can be very useful if we (as application developers) guarantee that native widgets will **never overlap** any alien widgets (except of their parent).

See this example:

```
MainWindow : QMainWindow ## native surface!
   ├── mdiArea : QMdiArea
   |    └── subWindow1 : QMdiSubWindow :::highlight ## native surface!
   ├── layersDocker : QDockWidget
   ├── colorSelectorDocker : QDockWidget
   ├── toolBox : QDockWidget
   └──  statusBar : QStatusBar
```

In this example `subWindow1` is the only widget of the hierarchy that has a native surface, hence it will be painted on the top of the dockers and the status bar. Even though, logically, it should be painted below them. This would be a bug, if we haven't used "tabbed" MDI mode, where the subwindow is automatically resized to be placed in between all the dockers and **never overlap** them. In this case, even though the rendering order is incorrect, the final visual representation is correct.

> [!WARNING]
> In the current version of Krita 6 on Wayland we have exactly this bug: if the user switches MDI interface into "Subwindows" mode and tries to move these subwindows over the MDI area, it is possible to move them over the docker area. We haven't yet decided what to do with that.

#### Qt's limitations in handling of the "relaxed widget ordering"

Since Qt expects all native widgets have native parents, the usage of "relaxed widget ordering" has some limitations. I.e. when a native widget is a child of an alien parent, and the user performs some manipulations with this alien parent (moves, resizes, raises or lowers), the updates may not propagate to the native child. That happens because alien widgets do not have any attached surface, so there is nothing to move, resize or reorder. So Qt optimizes this update out! It just marks the rect of this widget on the associated backing store (`QBackingStoreDefaultCompositor`) as dirty and exits.

This limitation can be illustrated with this set of widgets:

```
MainWindow : QMainWindow                ## native surface!
   └── mdiArea : QMdiArea               ## alien
        └── subWindow1 : QMdiSubWindow  ## alien
            └── view : KisView          ## alien
                └── ... : ...           ## alien
                    └── openGLCanvas : KisOpenGLCanvas2 ## native surface!
```

For example, when the user moves `subWindow1` over the MDI area, `QWidgetPrivate::setGeometry_sys()` is called. It checks if the widget is native or not. For native widget it forwards the call to `QPlatformWindow` to move the underlying surface. But for alien widgets it just marks the backing store buffer inside `MainWindow` as dirty and exits. **It doesn't expect alien widget to have any native child**, so the surface of the child will not be moved!

The same applies to subwindow resize, raise and lower operations. The operations are not forwarded to the children, because Qt doesn't expect them to be native [^3].

#### How Krita handles the "relaxed widget ordering" limitations

Here in Krita we resolve the issue of subwindow manipulations by forcing the subwindow widget to be a native surface as well. In this case, three of the four possible user requests (move, raise and lower) are fully handled by the window manager. And resize operation is forced on the native child by the parent-resize events delivered from the window manager as well.

The final widget hierarchy for the Krita canvas looks like that:

```
MainWindow : QMainWindow                ## native surface!
   └── mdiArea : QMdiArea               ## alien
        └── subWindow1 : QMdiSubWindow  ## **native surface!**
            └── view : KisView          ## alien
                └── canvasController : KisCanvasController          ## alien
                    └── viewport : KoCanvasControllerWidgetViewport ## alien
                        └── openGLCanvas : KisOpenGLCanvas2         ## native surface!
```

### Performance considerations when using native surfaces

Qt's design does not really expect widgets to be native, so it handles their updates quite badly. Each **toplevel** window has its own repaint manager (`QWidgetRepaintManager`). This single repaint manager handles the requests of **all** native widgets that belong to this toplevel window.

The dirty requests arrive via `QWidgetRepaintManager::markDirty()`, then then are posted into update queue in `QWidgetRepaintManager::sendUpdateRequest()`. In the upstream version of Qt6,
the update events are directly posted into the update queue, with a rudimentary rate-limit of 60 fps. When this implementation is used, the 60 fps bandwidth is basically **shared** between all the native widgets of the application, causing the canvas to be updated on every odd frame and the dockers on every even frame, basically halving the effective FPS rate of the canvas.

Here in Krita we resolve this issue in a two-fold way:

1) We reduce concurrency between native widget updates in Krita itself. Basically, when the user paints on or hovers over the canvas, dockers should not request any updates. That causes updates coming for a single surface, the 60 fps bandwidth is not shared, and the user gets full FPS.

   This solution does not require patching Qt (and hence will work for version of Krita for Linux distributions). But it has an obvious limitation: if the UIX logic requires Krita to update both, the canvas and the dockers simultaneously, the FPS drop will still happen. This happens, e.g. when rulers are visible and the user hovers over the image. Both surfaces have to be updated at the same time and Qt cannot handle it properly.

2) We patch `QWidgetRepaintManager` inside our version of Qt and implement a proper updates queue right inside the repaint manager. With this patch, the repaint manager synchronizes with the framerate of the display, gathers all updates for all native widgets in one batch and issues them with a controlled frame rate. It also implements a feature of controlled frame drops, i.e. if the previous frame hasn't completed its execution on the GPU, then no frame will be sent to GPU on the top of it. See the code in `QWidgetRepaintManager::slotCompressedUpdate()` for more details ([link](https://github.com/dimula73/qtbase/blob/for-krita/6.11.0/src/widgets/kernel/qwidgetrepaintmanager.cpp#L439C6-L439C51))

### Footnotes

[^1]: Technically, there are also "foreign" windows. These are "native" windows, which were created outside Qt and then embedded into `QWidget` hierarchy by creating a special wrapper `QWidget` with `QWidget::createWindowContainer()`

[^2]: There are shortcuts possible if we relax the requirements a bit, see below

[^3]: With a minor exception that resize operation sometimes propagates the resize operation to the child native surface via cache-reset of the layout of the parent.
