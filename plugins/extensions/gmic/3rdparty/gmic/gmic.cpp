/*
 #
 #  File        : gmic.cpp
 #                ( C++ source file )
 #
 #  Description : GREYC's Magic for Image Computing - G'MIC Language interpreter
 #                ( http://gmic.eu )
 #
 #  Copyright   : David Tschumperle
 #                ( http://tschumperle.users.greyc.fr/ )
 #
 #  License     : CeCILL v2.0
 #                ( http://www.cecill.info/licences/Licence_CeCILL_V2-en.html )
 #
 #  This software is governed by the CeCILL  license under French law and
 #  abiding by the rules of distribution of free software.  You can  use,
 #  modify and/ or redistribute the software under the terms of the CeCILL
 #  license as circulated by CEA, CNRS and INRIA at the following URL
 #  "http://www.cecill.info".
 #
 #  As a counterpart to the access to the source code and  rights to copy,
 #  modify and redistribute granted by the license, users are provided only
 #  with a limited warranty  and the software's author,  the holder of the
 #  economic rights,  and the successive licensors  have only  limited
 #  liability.
 #
 #  In this respect, the user's attention is drawn to the risks associated
 #  with loading,  using,  modifying and/or developing or reproducing the
 #  software by the user in light of its specific status of free software,
 #  that may mean  that it is complicated to manipulate,  and  that  also
 #  therefore means  that it is reserved for developers  and  experienced
 #  professionals having in-depth computer knowledge. Users are therefore
 #  encouraged to load and test the software's suitability as regards their
 #  requirements in conditions enabling the security of their systems and/or
 #  data to be ensured and,  more generally, to use and operate it in the
 #  same conditions as regards security.
 #
 #  The fact that you are presently reading this means that you have had
 #  knowledge of the CeCILL license and that you accept its terms.
 #
*/

// Add G'MIC-specific methods to the CImg library.
//------------------------------------------------
#ifdef cimg_plugin

// Additional arithmetic/boolean operators.
CImg<T>& gmic_invert_endianness(const char *const stype) {

#define _gmic_invert_endianness(value_type,svalue_type) \
  if (!std::strcmp(stype,svalue_type)) \
    if (cimg::type<T>::string()==cimg::type<value_type>::string()) invert_endianness(); \
    else CImg<value_type>(*this).invert_endianness().move_to(*this);
  _gmic_invert_endianness(unsigned char,"uchar")
  else _gmic_invert_endianness(unsigned char,"unsigned char")
    else _gmic_invert_endianness(char,"char")
      else _gmic_invert_endianness(unsigned short,"ushort")
        else _gmic_invert_endianness(unsigned short,"unsigned short")
          else _gmic_invert_endianness(short,"short")
            else _gmic_invert_endianness(unsigned int,"uint")
              else _gmic_invert_endianness(unsigned int,"unsigned int")
                else _gmic_invert_endianness(int,"int")
                  else _gmic_invert_endianness(unsigned int,"ulong")
                    else _gmic_invert_endianness(unsigned int,"unsigned long")
                      else _gmic_invert_endianness(int,"long")
                        else _gmic_invert_endianness(float,"float")
                          else _gmic_invert_endianness(double,"double")
                            else invert_endianness();
  return *this;
}

CImg<T> get_gmic_invert_endianness(const char *const stype) const {
  return (+*this).gmic_invert_endianness(stype);
}

template<typename t>
CImg<T>& operator_eq(const t val) {
#ifdef cimg_use_openmp
#pragma omp parallel for cimg_openmp_if (size()>=131072)
#endif
  cimg_rof(*this,ptrd,T) *ptrd = (T)(*ptrd == (T)val);
  return *this;
}

CImg<T>& operator_eq(const char *const expression) {
  return operator_eq((+*this)._fill(expression,true,true,0,0,"operator_eq",this));
}

template<typename t>
CImg<T>& operator_eq(const CImg<t>& img) {
  const unsigned long siz = size(), isiz = img.size();
  if (siz && isiz) {
    if (is_overlapped(img)) return operator_eq(+img);
    T *ptrd = _data, *const ptre = _data + siz;
    if (siz>isiz)
      for (unsigned long n = siz/isiz; n; --n)
        for (const t *ptrs = img._data, *ptrs_end = ptrs + isiz; ptrs<ptrs_end; ++ptrd)
          *ptrd = (T)(*ptrd == (T)*(ptrs++));
    for (const t *ptrs = img._data; ptrd<ptre; ++ptrd) *ptrd = (T)(*ptrd == (T)*(ptrs++));
  }
  return *this;
}

template<typename t>
CImg<T>& operator_neq(const t val) {
#ifdef cimg_use_openmp
#pragma omp parallel for cimg_openmp_if (size()>=131072)
#endif
  cimg_rof(*this,ptrd,T) *ptrd = (T)(*ptrd != (T)val);
  return *this;
}

CImg<T>& operator_neq(const char *const expression) {
  return operator_neq((+*this)._fill(expression,true,true,0,0,"operator_neq",this));
}

template<typename t>
CImg<T>& operator_neq(const CImg<t>& img) {
  const unsigned long siz = size(), isiz = img.size();
  if (siz && isiz) {
    if (is_overlapped(img)) return operator_neq(+img);
    T *ptrd = _data, *const ptre = _data + siz;
    if (siz>isiz)
      for (unsigned long n = siz/isiz; n; --n)
        for (const t *ptrs = img._data, *ptrs_end = ptrs + isiz; ptrs<ptrs_end; ++ptrd)
          *ptrd = (T)(*ptrd != (T)*(ptrs++));
    for (const t *ptrs = img._data; ptrd<ptre; ++ptrd) *ptrd = (T)(*ptrd != (T)*(ptrs++));
  }
  return *this;
}

template<typename t>
CImg<T>& operator_gt(const t val) {
#ifdef cimg_use_openmp
#pragma omp parallel for cimg_openmp_if (size()>=131072)
#endif
  cimg_rof(*this,ptrd,T) *ptrd = (T)(*ptrd > (T)val);
  return *this;
}

CImg<T>& operator_gt(const char *const expression) {
  return operator_gt((+*this)._fill(expression,true,true,0,0,"operator_gt",this));
}

template<typename t>
CImg<T>& operator_gt(const CImg<t>& img) {
  const unsigned long siz = size(), isiz = img.size();
  if (siz && isiz) {
    if (is_overlapped(img)) return operator_gt(+img);
    T *ptrd = _data, *const ptre = _data + siz;
    if (siz>isiz)
      for (unsigned long n = siz/isiz; n; --n)
        for (const t *ptrs = img._data, *ptrs_end = ptrs + isiz; ptrs<ptrs_end; ++ptrd)
          *ptrd = (T)(*ptrd > (T)*(ptrs++));
    for (const t *ptrs = img._data; ptrd<ptre; ++ptrd) *ptrd = (T)(*ptrd > (T)*(ptrs++));
  }
  return *this;
}

template<typename t>
CImg<T>& operator_ge(const t val) {
#ifdef cimg_use_openmp
#pragma omp parallel for cimg_openmp_if (size()>=131072)
#endif
  cimg_rof(*this,ptrd,T) *ptrd = (T)(*ptrd >= (T)val);
  return *this;
}

CImg<T>& operator_ge(const char *const expression) {
  return operator_ge((+*this)._fill(expression,true,true,0,0,"operator_ge",this));
}

template<typename t>
CImg<T>& operator_ge(const CImg<t>& img) {
  const unsigned long siz = size(), isiz = img.size();
  if (siz && isiz) {
    if (is_overlapped(img)) return operator_ge(+img);
    T *ptrd = _data, *const ptre = _data + siz;
    if (siz>isiz)
      for (unsigned long n = siz/isiz; n; --n)
        for (const t *ptrs = img._data, *ptrs_end = ptrs + isiz; ptrs<ptrs_end; ++ptrd)
          *ptrd = (T)(*ptrd >= (T)*(ptrs++));
    for (const t *ptrs = img._data; ptrd<ptre; ++ptrd) *ptrd = (T)(*ptrd >= (T)*(ptrs++));
  }
  return *this;
}

template<typename t>
CImg<T>& operator_lt(const t val) {
#ifdef cimg_use_openmp
#pragma omp parallel for cimg_openmp_if (size()>=131072)
#endif
  cimg_rof(*this,ptrd,T) *ptrd = (T)(*ptrd < (T)val);
  return *this;
}

CImg<T>& operator_lt(const char *const expression) {
  return operator_lt((+*this)._fill(expression,true,true,0,0,"operator_lt",this));
}

template<typename t>
CImg<T>& operator_lt(const CImg<t>& img) {
  const unsigned long siz = size(), isiz = img.size();
  if (siz && isiz) {
    if (is_overlapped(img)) return operator_lt(+img);
    T *ptrd = _data, *const ptre = _data + siz;
    if (siz>isiz)
      for (unsigned long n = siz/isiz; n; --n)
        for (const t *ptrs = img._data, *ptrs_end = ptrs + isiz; ptrs<ptrs_end; ++ptrd)
          *ptrd = (T)(*ptrd < (T)*(ptrs++));
    for (const t *ptrs = img._data; ptrd<ptre; ++ptrd) *ptrd = (T)(*ptrd < (T)*(ptrs++));
  }
  return *this;
}

template<typename t>
CImg<T>& operator_le(const t val) {
#ifdef cimg_use_openmp
#pragma omp parallel for cimg_openmp_if (size()>=131072)
#endif
  cimg_rof(*this,ptrd,T) *ptrd = (T)(*ptrd <= (T)val);
  return *this;
}

CImg<T>& operator_le(const char *const expression) {
  return operator_le((+*this)._fill(expression,true,true,0,0,"operator_le",this));
}

template<typename t>
CImg<T>& operator_le(const CImg<t>& img) {
  const unsigned long siz = size(), isiz = img.size();
  if (siz && isiz) {
    if (is_overlapped(img)) return operator_le(+img);
    T *ptrd = _data, *const ptre = _data + siz;
    if (siz>isiz)
      for (unsigned long n = siz/isiz; n; --n)
        for (const t *ptrs = img._data, *ptrs_end = ptrs + isiz; ptrs<ptrs_end; ++ptrd)
          *ptrd = (T)(*ptrd <= (T)*(ptrs++));
    for (const t *ptrs = img._data; ptrd<ptre; ++ptrd) *ptrd = (T)(*ptrd <= (T)*(ptrs++));
  }
  return *this;
}

CImg<T>& mul(const char *const expression) {
  return operator*=(expression);
}

CImg<T> get_mul(const char *const expression) const {
  return (+*this).mul(expression);
}

CImg<T>& div(const char *const expression) {
  return operator/=(expression);
}

CImg<T> get_div(const char *const expression) const {
  return (+*this).div(expression);
}

CImg<T>& gmic_discard(const char *const axes) {
  for (const char *s = axes; *s; ++s) discard(*s);
  return *this;
}

CImg<T> get_gmic_discard(const char *const axes) const {
  return (+*this).gmic_discard(axes);
}

template<typename t>
CImg<T>& gmic_discard(const CImg<t>& values, const char *const axes) {
  if (is_empty() || !values || !axes || !*axes) return *this;
  for (const char *s = axes; *s; ++s) discard(values,*s);
  return *this;
}

template<typename t>
CImg<T> get_gmic_discard(const CImg<t>& values, const char *const axes) const {
  return (+*this).gmic_discard(values,axes);
}

template<typename t>
const CImg<T>& gmic_symmetric_eigen(CImg<t>& val, CImg<t>& vec) const {
  if (spectrum()!=3 && spectrum()!=6) return symmetric_eigen(val,vec);
  val.assign(width(),height(),depth(),spectrum()==3?2:3);
  vec.assign(width(),height(),depth(),spectrum()==3?2:6);
  CImg<t> _val, _vec;
  cimg_forXYZ(*this,x,y,z) {
    get_tensor_at(x,y,z).symmetric_eigen(_val,_vec);
    val.set_vector_at(_val,x,y,z);
    if (spectrum()==3) {
      vec(x,y,z,0) = _vec(0,0);
      vec(x,y,z,1) = _vec(0,1);
    } else {
      vec(x,y,z,0) = _vec(0,0);
      vec(x,y,z,1) = _vec(0,1);
      vec(x,y,z,2) = _vec(0,2);

      vec(x,y,z,3) = _vec(1,0);
      vec(x,y,z,4) = _vec(1,1);
      vec(x,y,z,5) = _vec(1,2);
    }
  }
  return *this;
}

// Additional geometric and drawing operators.
CImg<T>& append_string_to(CImg<T>& img) const {
  const unsigned int w = img._width;
  CImg<T> res(w + _width);
  std::memcpy(res._data,img._data,w*sizeof(T));
  std::memcpy(res._data + w,_data,_width*sizeof(T));
  return res.move_to(img);
}

template<typename t>
CImg<T>& replace(CImg<t>& img) {
  return img.move_to(*this);
}

template<typename t>
CImg<T> get_replace(const CImg<t>& img) const {
  return +img;
}

CImg<T>& gmic_autocrop(const CImg<T>& color=CImg<T>::empty()) {
  if (color.width()==1) autocrop(*color);
  else autocrop(color);
  return *this;
}

CImg<T> get_gmic_autocrop(const CImg<T>& color=CImg<T>::empty()) {
  return (+*this).gmic_autocrop(color);
}

CImg<T> get_gmic_set(const double value,
                     const int x, const int y, const int z, const int v) const {
  return (+*this).gmic_set(value,x,y,z,v);
}

CImg<T>& gmic_set(const double value,
                  const int x, const int y, const int z, const int v) {
  (*this).atXYZC(x,y,z,v,0) = (T)value;
  return *this;
}

CImg<T> get_draw_point(const int x, const int y, const int z, const T *const col,
                       const float opacity) const {
  return (+*this).draw_point(x,y,z,col,opacity);
}

CImg<T> get_draw_line(const int x0, const int y0, const int x1, const int y1, const T *const col,
                      const float opacity, const unsigned int pattern) const {
  return (+*this).draw_line(x0,y0,x1,y1,col,opacity,pattern);
}

template<typename t>
CImg<T> get_draw_polygon(const CImg<t>& pts, const T *const col, const float opacity) const {
  return (+*this).draw_polygon(pts,col,opacity);
}

template<typename t>
CImg<T> get_draw_polygon(const CImg<t>& pts, const T *const col, const float opacity,
                         const unsigned int pattern) const {
  return (+*this).draw_polygon(pts,col,opacity,pattern);
}

CImg<T> get_draw_circle(const int x, const int y, const int r, const T *const col,
                        const float opacity) const {
  return (+*this).draw_circle(x,y,r,col,opacity);
}

CImg<T> get_draw_circle(const int x, const int y, const int r, const T *const col,
                        const float opacity,
                        const unsigned int pattern) const {
  return (+*this).draw_circle(x,y,r,col,opacity,pattern);
}

CImg<T> get_draw_ellipse(const int x, const int y, const float r0, const float r1,
                         const float angle, const T *const col, const float opacity) const {
  return (+*this).draw_ellipse(x,y,r0,r1,angle,col,opacity);
}

CImg<T> get_draw_ellipse(const int x, const int y, const float r0, const float r1,
                         const float angle, const T *const col, const float opacity,
                         const unsigned int pattern) const {
  return (+*this).draw_ellipse(x,y,r0,r1,angle,col,opacity,pattern);
}

CImg<T>& gmic_draw_text(const int x, const int y,
                        const char *const text, const T *const col,
                        const int bg, const float opacity, const unsigned int siz,
                        const unsigned int nb_cols) {
  if (is_empty()) {
    const T one[] = { 1 };
    assign().draw_text(x,y,"%s",one,0,opacity,siz,text).resize(-100,-100,1,nb_cols);
    cimg_forC(*this,c) get_shared_channel(c)*=col[c];
  } else draw_text(x,y,"%s",col,bg,opacity,siz,text);
  return *this;
}

CImg<T> get_gmic_draw_text(const int x, const int y,
                           const char *const text, const T *const col,
                           const int bg, const float opacity, const unsigned int siz,
                           const unsigned int nb_cols) const {
  return (+*this).gmic_draw_text(x,y,text,col,bg,opacity,siz,nb_cols);
}

CImg<T> get_draw_image(const int x, const int y, const int z, const int c,
                       const CImg<T>& sprite, const CImg<T>& mask, const float opacity,
                       const float max_opacity_mask) const {
  return (+*this).draw_image(x,y,z,c,sprite,mask,opacity,max_opacity_mask);
}

CImg<T> get_draw_image(const int x, const int y, const int z, const int c,
                       const CImg<T>& sprite, const float opacity) const {
  return (+*this).draw_image(x,y,z,c,sprite,opacity);
}

CImg<T> get_draw_plasma(const float alpha, const float beta, const unsigned int scale) const {
  return (+*this).draw_plasma(alpha,beta,scale);
}

CImg<T> get_draw_mandelbrot(const CImg<T>& color_palette, const float opacity,
                            const double z0r, const double z0i, const double z1r, const double z1i,
                            const unsigned int itermax, const bool normalized_iteration,
                            const bool julia_set, const double paramr, const double parami) const {
  return (+*this).draw_mandelbrot(color_palette,opacity,z0r,z0i,z1r,z1i,itermax,
                                  normalized_iteration,julia_set,paramr,parami);
}

template<typename t, typename tc>
CImg<T> get_draw_graph(const CImg<t>& data,
                       const tc *const color, const float opacity=1,
                       const unsigned int plot_type=1, const int vertex_type=1,
                       const double ymin=0, const double ymax=0,
                       const unsigned int pattern=~0U) const {
  return (+*this).draw_graph(data,color,opacity,plot_type,vertex_type,ymin,ymax,pattern);
}

template<typename tc>
CImg<T> get_draw_axes(const float x0, const float x1, const float y0, const float y1,
                      const tc *const color, const float opacity=1,
                      const int subdivisionx=-60, const int subdivisiony=-60,
                      const float precisionx=0, const float precisiony=0,
                      const unsigned int patternx=~0U, const unsigned int patterny=~0U,
                      const unsigned int font_height=13) const {
  return (+*this).draw_axes(x0,x1,y0,y1,color,opacity,subdivisionx,subdivisiony,
                            precisionx,precisiony,patternx,patterny,font_height);
}

template<typename tc>
CImg<T> get_draw_grid(const float sizex, const float sizey,
                      const float offsetx, const float offsety,
                      const bool invertx, const bool inverty,
                      const tc *const color, const float opacity=1,
                      const unsigned int patternx=~0U, const unsigned int patterny=~0U) {
  return (+*this).draw_grid(sizex,sizey,offsetx,offsety,invertx,inverty,color,opacity,
                            patternx,patterny);
}

template<typename t1, typename t2>
CImg<T> get_draw_quiver(const CImg<t1>& flow,
                        const t2 *const color, const float opacity=1,
                        const unsigned int sampling=25, const float factor=-20,
                        const bool arrows=true, const unsigned int pattern=~0U) const {
  return (+*this).draw_quiver(flow,color,opacity,sampling,factor,arrows,pattern);
}

CImg<T> get_draw_fill(const int x, const int y, const int z,
                      const T *const col, const float opacity,
                      const float tolerance, const bool is_high_connectivity) const {
  return (+*this).draw_fill(x,y,z,col,opacity,tolerance,is_high_connectivity);
}

static bool is_almost(const T x, const T c) {
  return x>=c && x<c+1;
}

template<typename tp, typename tf, typename tc, typename to>
CImg<T> get_draw_object3d(const float x0, const float y0, const float z0,
                          const CImg<tp>& vertices, const CImgList<tf>& primitives,
                          const CImgList<tc>& colors, const CImgList<to>& opacities,
                          const unsigned int render_mode, const bool double_sided,
                          const float focale,
                          const float light_x, const float light_y,const float light_z,
                          const float specular_light, const float specular_shine,
                          CImg<floatT>& zbuffer) const {
  return (+*this).draw_object3d(x0,y0,z0,vertices,primitives,colors,opacities,render_mode,
                                double_sided,focale,light_x,light_y,light_z,specular_light,
                                specular_shine,zbuffer);
}

// Additional 3d objects operators.

CImgList<T> get_split_CImg3d() const {
  CImg<charT> error_message(1024);
  if (!is_CImg3d(false,error_message))
    throw CImgInstanceException(_cimg_instance
                                "get_split_CImg3d(): image instance is not a CImg3d (%s).",
                                cimg_instance,error_message.data());
  CImgList<T> res;
  const T *ptr0 = _data, *ptr = ptr0 + 6;
  CImg<T>(ptr0,1,(unsigned int)(ptr - ptr0),1,1).move_to(res); // Header.
  ptr0 = ptr;
  const unsigned int
    nbv = cimg::float2uint(*(ptr++)),
    nbp = cimg::float2uint(*(ptr++));
  CImg<T>(ptr0,1,(unsigned int)(ptr - ptr0),1,1).move_to(res); // Nb vertices and primitives.
  ptr0 = ptr; ptr+=3*nbv;
  CImg<T>(ptr0,1,(unsigned int)(ptr - ptr0),1,1).move_to(res); // Vertices.
  ptr0 = ptr;
  for (unsigned int i = 0; i<nbp; ++i) ptr+=(unsigned int)(*ptr) + 1;
  CImg<T>(ptr0,1,(unsigned int)(ptr - ptr0),1,1).move_to(res); // Primitives.
  ptr0 = ptr;
  for (unsigned int i = 0; i<nbp; ++i) {
    const T val = *(ptr++);
    if (val!=-128) ptr+=2;
    else {
      const unsigned int
        w = cimg::float2uint(*(ptr++)),
        h = cimg::float2uint(*(ptr++)),
        s = cimg::float2uint(*(ptr++));
      if (w*h*s!=0) ptr+=w*h*s;
    }
  }
  CImg<T>(ptr0,1,(unsigned int)(ptr - ptr0),1,1).move_to(res); // Colors/Textures.
  ptr0 = ptr;
  for (unsigned int i = 0; i<nbp; ++i) {
    const T val = *(ptr++);
    if (val==-128) {
      const unsigned int
        w = cimg::float2uint(*(ptr++)),
        h = cimg::float2uint(*(ptr++)),
        s = cimg::float2uint(*(ptr++));
      if (w*h*s!=0) ptr+=w*h*s;
    }
  }
  CImg<T>(ptr0,1,(unsigned int)(ptr - ptr0),1,1).move_to(res); // Opacities.
  return res;
}

static CImg<T> append_CImg3d(const CImgList<T>& images) {
  if (!images) return CImg<T>();
  if (images.size()==1) return +images[0];
  CImg<charT> error_message(1024);
  unsigned int nbv = 0, nbp = 0;
  unsigned long siz = 0;
  cimglist_for(images,l) {
    const CImg<T>& img = images[l];
    if (!img.is_CImg3d(false,error_message))
      throw CImgArgumentException("append_CImg3d(): image [%d] (%u,%u,%u,%u,%p) "
                                  "is not a CImg3d (%s).",
                                  l,img._width,img._height,img._depth,img._spectrum,img._data,
                                  error_message.data());
    siz+=img.size() - 8;
    nbv+=cimg::float2uint((float)img[6]);
    nbp+=cimg::float2uint((float)img[7]);
  }

  CImg<T> res(1,siz + 8);
  const T **const ptrs = new const T*[images.size()];
  T *ptrd = res._data;
  *(ptrd++) = (T)('C' + 0.5f); *(ptrd++) = (T)('I' + 0.5f); // Create object header.
  *(ptrd++) = (T)('m' + 0.5f); *(ptrd++) = (T)('g' + 0.5f);
  *(ptrd++) = (T)('3' + 0.5f); *(ptrd++) = (T)('d' + 0.5f);
  *(ptrd++) = (T)cimg::uint2float(nbv);
  *(ptrd++) = (T)cimg::uint2float(nbp);
  cimglist_for(images,l) { // Merge object points.
    const CImg<T>& img = images[l];
    const unsigned int nbv = cimg::float2uint((float)img[6]);
    std::memcpy(ptrd,img._data + 8,3*nbv*sizeof(T));
    ptrd+=3*nbv;
    ptrs[l] = img._data + 8 + 3*nbv;
  }
  unsigned long poff = 0;
  cimglist_for(images,l) { // Merge object primitives.
    const unsigned int
      nbv = cimg::float2uint((float)images[l][6]),
      nbp = cimg::float2uint((float)images[l][7]);
    for (unsigned int p = 0; p<nbp; ++p) {
      const unsigned int
        nbi = cimg::float2uint((float)*(ptrs[l]++)),
        _nbi = nbi<5?nbi:nbi==5?2:nbi/3;
      *(ptrd++) = (T)cimg::uint2float(nbi);
      for (unsigned int i = 0; i<_nbi; ++i) *(ptrd++) = (T)cimg::uint2float(cimg::float2uint(*(ptrs[l]++)) + poff);
      for (unsigned int i = nbi - _nbi; i; --i) *(ptrd++) = *(ptrs[l]++);
    }
    poff+=nbv;
  }
  unsigned long voff = 0;
  cimglist_for(images,l) { // Merge object colors.
    const unsigned int nbc = cimg::float2uint((float)images[l][7]);
    for (unsigned int c = 0; c<nbc; ++c)
      if (*(ptrs[l])==(T)-128) {
        *(ptrd++) = *(ptrs[l]++);
        const unsigned int
          w = (unsigned int)*(ptrs[l]++),
          h = (unsigned int)*(ptrs[l]++),
          s = (unsigned int)*(ptrs[l]++);
        if (!h && !s) { *(ptrd++) = (T)(w + voff); *(ptrd++) = 0; *(ptrd++) = 0; }
        else {
          *(ptrd++) = (T)w; *(ptrd++) = (T)h; *(ptrd++) = (T)s;
          const unsigned long whs = (unsigned long)w*h*s;
          std::memcpy(ptrd,ptrs[l],whs*sizeof(T));
          ptrs[l]+=whs; ptrd+=whs;
        }
      } else { *(ptrd++) = *(ptrs[l]++); *(ptrd++) = *(ptrs[l]++); *(ptrd++) = *(ptrs[l]++); }
    voff+=nbc;
  }
  voff = 0;
  cimglist_for(images,l) { // Merge object opacities.
    const unsigned int nbo = cimg::float2uint((float)images[l][7]);
    for (unsigned int o = 0; o<nbo; ++o)
      if (*(ptrs[l])==(T)-128) {
        *(ptrd++) = *(ptrs[l]++);
        const unsigned int
          w = (unsigned int)*(ptrs[l]++),
          h = (unsigned int)*(ptrs[l]++),
          s = (unsigned int)*(ptrs[l]++);
        if (!h && !s) { *(ptrd++) = (T)(w + voff); *(ptrd++) = 0; *(ptrd++) = 0; }
        else {
          *(ptrd++) = (T)w; *(ptrd++) = (T)h; *(ptrd++) = (T)s;
          const unsigned long whs = (unsigned long)w*h*s;
          std::memcpy(ptrd,ptrs[l],whs*sizeof(T));
          ptrs[l]+=whs; ptrd+=whs;
        }
      } else *(ptrd++) = *(ptrs[l]++);
    voff+=nbo;
  }
  delete[] ptrs;
  return res;
}

template<typename t>
CImg<T>& rotate_CImg3d(const CImg<t>& rot) {
  CImg<charT> error_message(1024);
  if (!is_CImg3d(false,error_message))
    throw CImgInstanceException(_cimg_instance
                                "rotate_CImg3d(): image instance is not a CImg3d (%s).",
                                cimg_instance,error_message.data());
  const unsigned int nbv = cimg::float2uint((float)(*this)[6]);
  const T *ptrs = data() + 8;
  const float
    a = (float)rot(0,0), b = (float)rot(1,0), c = (float)rot(2,0),
    d = (float)rot(0,1), e = (float)rot(1,1), f = (float)rot(2,1),
    g = (float)rot(0,2), h = (float)rot(1,2), i = (float)rot(2,2);
  T *ptrd = data() + 8;
  for (unsigned int j = 0; j<nbv; ++j) {
    const float x = (float)*(ptrs++), y = (float)*(ptrs++), z = (float)*(ptrs++);
    *(ptrd++) = (T)(a*x + b*y + c*z);
    *(ptrd++) = (T)(d*x + e*y + f*z);
    *(ptrd++) = (T)(g*x + h*y + i*z);
  }
  return *this;
}

template<typename t>
CImg<T> get_rotate_CImg3d(const CImg<t>& rot) const {
  return (+*this).rotate_CImg3d(rot);
}

CImg<T>& gmic_shift(const float delta_x, const float delta_y=0, const float delta_z=0, const float delta_c=0,
                    const int boundary_conditions=0) {
  const int idelta_x = (int)delta_x, idelta_y = (int)delta_y, idelta_z = (int)delta_z, idelta_c = (int)delta_c;
  if (delta_x==(float)idelta_x && delta_y==(float)idelta_y && delta_z==(float)idelta_z && delta_c==(float)idelta_c) {
    // Integer displacement.
    return shift(idelta_x,idelta_y,idelta_z,idelta_c,boundary_conditions);
  }
  // Non-integer displacement.
  return _get_gmic_shift(delta_x,delta_y,delta_z,delta_c,boundary_conditions).move_to(*this);
}

// [note] get_gmic_shift() as it is avoids an additional buffer when non-integer displacement is used.
CImg<T> get_gmic_shift(const float delta_x, const float delta_y=0, const float delta_z=0, const float delta_c=0,
                       const int boundary_conditions=0) const {
  const int idelta_x = (int)delta_x, idelta_y = (int)delta_y, idelta_z = (int)delta_z, idelta_c = (int)delta_c;
  if (delta_x==(float)idelta_x && delta_y==(float)idelta_y && delta_z==(float)idelta_z && delta_c==(float)idelta_c) {
    return (+*this).shift(idelta_x,idelta_y,idelta_z,idelta_c,boundary_conditions); // Integer displacement.
  }
  return _get_gmic_shift(delta_x,delta_y,delta_z,delta_c,boundary_conditions); // Non-integer displacement.
}

CImg<T> _get_gmic_shift(const float delta_x, const float delta_y=0, const float delta_z=0, const float delta_c=0,
                        const int boundary_conditions=0) const {
  CImg<T> res(_width,_height,_depth,_spectrum);
  if (delta_c!=0) // 4D shift.
    switch (boundary_conditions) {
    case 0: // Dirichlet.
#ifdef cimg_use_openmp
#pragma omp parallel for collapse(3) if (res.size()>=4096)
#endif
      cimg_forXYZC(res,x,y,z,c) res(x,y,z,c) = linear_atXYZC(x - delta_x,y - delta_y,z - delta_z,c - delta_c,0);
      break;
    case 1: // Neumann.
#ifdef cimg_use_openmp
#pragma omp parallel for collapse(3) if (res.size()>=4096)
#endif
      cimg_forXYZC(res,x,y,z,c) res(x,y,z,c) = linear_atXYZC(x - delta_x,y - delta_y,z - delta_z,c - delta_c);
      break;
    default: // Periodic.
#ifdef cimg_use_openmp
#pragma omp parallel for collapse(3) if (res.size()>=4096)
#endif
      cimg_forXYZC(res,x,y,z,c) res(x,y,z,c) = linear_atXYZC(cimg::mod(x - delta_x,(float)_width),
                                                             cimg::mod(y - delta_y,(float)_height),
                                                             cimg::mod(z - delta_z,(float)_depth),
                                                             cimg::mod(c - delta_c,(float)_spectrum));
    }
  else if (delta_z!=0) // 3D shift.
    switch (boundary_conditions) {
    case 0: // Dirichlet.
#ifdef cimg_use_openmp
#pragma omp parallel for collapse(3) if (res.size()>=4096)
#endif
      cimg_forC(res,c) cimg_forXYZ(res,x,y,z) res(x,y,z,c) = linear_atXYZ(x - delta_x,y - delta_y,z - delta_z,c,0);
      break;
    case 1: // Neumann.
#ifdef cimg_use_openmp
#pragma omp parallel for collapse(3) if (res.size()>=4096)
#endif
      cimg_forC(res,c) cimg_forXYZ(res,x,y,z) res(x,y,z,c) = linear_atXYZ(x - delta_x,y - delta_y,z - delta_z,c);
      break;
    default: // Periodic.
#ifdef cimg_use_openmp
#pragma omp parallel for collapse(3) if (res.size()>=4096)
#endif
      cimg_forC(res,c) cimg_forXYZ(res,x,y,z) res(x,y,z,c) = linear_atXYZ(cimg::mod(x - delta_x,(float)_width),
                                                                          cimg::mod(y - delta_y,(float)_height),
                                                                          cimg::mod(z - delta_z,(float)_depth),c);
    }
  else if (delta_y!=0) // 2D shift.
    switch (boundary_conditions) {
    case 0: // Dirichlet.
#ifdef cimg_use_openmp
#pragma omp parallel for collapse(3) if (res.size()>=4096)
#endif
      cimg_forZC(res,z,c) cimg_forXY(res,x,y) res(x,y,z,c) = linear_atXY(x - delta_x,y - delta_y,z,c,0);
      break;
    case 1: // Neumann.
#ifdef cimg_use_openmp
#pragma omp parallel for collapse(3) if (res.size()>=4096)
#endif
      cimg_forZC(res,z,c) cimg_forXY(res,x,y) res(x,y,z,c) = linear_atXY(x - delta_x,y - delta_y,z,c);
      break;
    default: // Periodic.
#ifdef cimg_use_openmp
#pragma omp parallel for collapse(3) if (res.size()>=4096)
#endif
      cimg_forZC(res,z,c) cimg_forXY(res,x,y) res(x,y,z,c) = linear_atXY(cimg::mod(x - delta_x,(float)_width),
                                                                         cimg::mod(y - delta_y,(float)_height),z,c);
    }
  else // 1D shift.
    switch (boundary_conditions) {
    case 0: // Dirichlet.
#ifdef cimg_use_openmp
#pragma omp parallel for collapse(3) if (res.size()>=4096)
#endif
      cimg_forYZC(res,y,z,c) cimg_forX(res,x) res(x,y,z,c) = linear_atX(x - delta_x,y,z,c,0);
      break;
    case 1: // Neumann.
#ifdef cimg_use_openmp
#pragma omp parallel for collapse(3) if (res.size()>=4096)
#endif
      cimg_forYZC(res,y,z,c) cimg_forX(res,x) res(x,y,z,c) = linear_atX(x - delta_x,y,z,c);
      break;
    default: // Periodic.
#ifdef cimg_use_openmp
#pragma omp parallel for collapse(3) if (res.size()>=4096)
#endif
      cimg_forYZC(res,y,z,c) cimg_forX(res,x) res(x,y,z,c) = linear_atX(cimg::mod(x - delta_x,(float)_width),y,z,c);
    }
  return res;
}

CImg<T>& shift_CImg3d(const float tx, const float ty, const float tz) {
  CImg<charT> error_message(1024);
  if (!is_CImg3d(false,error_message))
    throw CImgInstanceException(_cimg_instance
                                "shift_CImg3d(): image instance is not a CImg3d (%s).",
                                cimg_instance,error_message.data());
  const unsigned int nbv = cimg::float2uint((float)(*this)[6]);
  T *ptrd = data() + 8;
  for (unsigned int j = 0; j<nbv; ++j) { *(ptrd++)+=(T)tx; *(ptrd++)+=(T)ty; *(ptrd++)+=(T)tz; }
  return *this;
}

CImg<T> get_shift_CImg3d(const float tx, const float ty, const float tz) const {
  return (+*this).shift_CImg3d(tx,ty,tz);
}

CImg<T>& scale_CImg3d(const float sx, const float sy, const float sz) {
  CImg<charT> error_message(1024);
  if (!is_CImg3d(false,error_message))
    throw CImgInstanceException(_cimg_instance
                                "scale_CImg3d(): image instance is not a CImg3d (%s).",
                                cimg_instance,error_message.data());
  const unsigned int nbv = cimg::float2uint((float)(*this)[6]);
  T *ptrd = data() + 8;
  for (unsigned int j = 0; j<nbv; ++j) { *(ptrd++)*=(T)sx; *(ptrd++)*=(T)sy; *(ptrd++)*=(T)sz; }
  return *this;
}

CImg<T> get_scale_CImg3d(const float sx, const float sy, const float sz) const {
  return (+*this).scale_CImg3d(sx,sy,sz);
}

CImg<T>& reverse_CImg3d() {
  CImg<char> error_message(1024);
  if (!is_CImg3d(false,error_message))
    throw CImgInstanceException(_cimg_instance
                                "reverse_CImg3d(): image instance is not a CImg3d (%s).",
                                cimg_instance,error_message.data());
  T *p = _data + 6;
  const unsigned int
    nbv = cimg::float2uint(*(p++)),
    nbp = cimg::float2uint(*(p++));
  p+=3*nbv;
  for (unsigned int i = 0; i<nbp; ++i) {
    const unsigned int nb = (unsigned int)*(p++);
    switch(nb) {
    case 2: case 3: cimg::swap(p[0],p[1]); break;
    case 6: cimg::swap(p[0],p[1],p[2],p[4],p[3],p[5]); break;
    case 9: cimg::swap(p[0],p[1],p[3],p[5],p[4],p[6]); break;
    case 4: cimg::swap(p[0],p[1],p[2],p[3]); break;
    case 12: cimg::swap(p[0],p[1],p[2],p[3],p[4],p[6],p[5],p[7],p[8],p[10],p[9],p[11]); break;
    }
    p+=nb;
  }
  return *this;
}

CImg<T> get_reverse_CImg3d() const {
  return (+*this).reverse_CImg3d();
}

CImg<T>& color_CImg3d(const float R, const float G, const float B, const float opacity,
                      const bool set_RGB, const bool set_opacity) {
  CImg<char> error_message(1024);
  if (!is_CImg3d(false,error_message))
    throw CImgInstanceException(_cimg_instance
                                "color_CImg3d(): image instance is not a CImg3d (%s).",
                                cimg_instance,error_message.data());
  T *ptrd = data() + 6;
  const unsigned int
    nbv = cimg::float2uint((float)*(ptrd++)),
    nbp = cimg::float2uint((float)*(ptrd++));
  ptrd+=3*nbv;
  for (unsigned int i = 0; i<nbp; ++i) { const unsigned int N = (unsigned int)*(ptrd++); ptrd+=N; }
  for (unsigned int c = 0; c<nbp; ++c)
    if (*ptrd==(T)-128) {
      ++ptrd;
      const unsigned int
        w = (unsigned int)*(ptrd++),
        h = (unsigned int)*(ptrd++),
        s = (unsigned int)*(ptrd++);
      ptrd+=w*h*s;
    } else if (set_RGB) { *(ptrd++) = (T)R; *(ptrd++) = (T)G; *(ptrd++) = (T)B; } else ptrd+=3;
  if (set_opacity)
    for (unsigned int o = 0; o<nbp; ++o) {
      if (*ptrd==(T)-128) {
        ++ptrd;
        const unsigned int
          w = (unsigned int)*(ptrd++),
          h = (unsigned int)*(ptrd++),
          s = (unsigned int)*(ptrd++);
        ptrd+=w*h*s;
      } else *(ptrd++) = (T)opacity;
    }
  return *this;
}

CImg<T> get_color_CImg3d(const float R, const float G, const float B,
                         const float opacity, const bool set_RGB, const bool set_opacity) const {
  return (+*this).color_CImg3d(R,G,B,opacity,set_RGB,set_opacity);
}

CImg<T>& texturize_CImg3d(const CImg<T>& texture, const CImg<T>& coords) {
  return get_texturize_CImg3d(texture,coords).move_to(*this);
}

CImg<T> get_texturize_CImg3d(const CImg<T>& texture, const CImg<T>& coords) const {
  CImgList<uintT> primitives;
  CImgList<T> colors;
  CImgList<floatT> opacities;
  const CImg<floatT> points = get_CImg3dtoobject3d(primitives,colors,opacities,false);
  if (coords)
    points.texturize_object3d(primitives,colors,texture,coords.get_resize(2,coords.size()/2,1,1,-1).transpose());
  else points.texturize_object3d(primitives,colors,texture,coords);
  return points.get_object3dtoCImg3d(primitives,colors,opacities,false);
}

CImg<T>& convert_primitives_CImg3d(const unsigned int mode) {
  CImg<char> error_message(1024);
  if (!is_CImg3d(false,error_message))
    throw CImgInstanceException(_cimg_instance
                                "convert_primitives_CImg3d(): image instance is not a CImg3d (%s).",
                                cimg_instance,error_message.data());
  CImgList<uintT> primitives;
  CImgList<floatT> colors, opacities;
  CImg3dtoobject3d(primitives,colors,opacities,false);
  const unsigned int psiz = primitives.size();
  CImg<uintT> P;
  CImg<floatT> C, O;
  for (unsigned int p = 0; p<psiz; ++p) {
    primitives[p].move_to(P);
    colors[p].move_to(C);
    opacities[p].move_to(O);
    switch (P.size()) {
    case 1 : // Point.
      P.move_to(primitives);
      if (mode==2) {
        if (C.size()==3) C.move_to(colors);
        else C.get_vector_at(C._width/2,C._height/2).move_to(colors);
        if (O.size()==1) O.move_to(opacities);
        else O.get_vector_at(O._width/2,O._height/2).move_to(opacities);
      } else { C.move_to(colors); O.move_to(opacities); }
      break;
    case 2 : // Colored segment.
      if (mode) { P.move_to(primitives); C.move_to(colors); O.move_to(opacities); }
      else {
        CImg<uintT>::vector(P[0]).move_to(primitives);
        colors.insert(C); opacities.insert(O);
        CImg<uintT>::vector(P[1]).move_to(primitives);
        C.move_to(colors); O.move_to(opacities);
      }
      break;
    case 3 : // Colored triangle.
      if (mode==2) {
        P.move_to(primitives); C.move_to(colors); O.move_to(opacities);
      } else if (mode==1) {
        CImg<uintT>::vector(P[0],P[1]).move_to(primitives);
        colors.insert(C); opacities.insert(O);
        CImg<uintT>::vector(P[1],P[2]).move_to(primitives);
        colors.insert(C); opacities.insert(O);
        CImg<uintT>::vector(P[2],P[0]).move_to(primitives);
        C.move_to(colors); O.move_to(opacities);
      } else {
        CImg<uintT>::vector(P[0]).move_to(primitives);
        colors.insert(C); opacities.insert(O);
        CImg<uintT>::vector(P[1]).move_to(primitives);
        colors.insert(C); opacities.insert(O);
        CImg<uintT>::vector(P[2]).move_to(primitives);
        C.move_to(colors); O.move_to(opacities);
      }
      break;
    case 4 : // Colored quadrangle.
      if (mode==2) {
        P.move_to(primitives); C.move_to(colors); O.move_to(opacities);
      } else if (mode==1) {
        CImg<uintT>::vector(P[0],P[1]).move_to(primitives);
        colors.insert(C); opacities.insert(O);
        CImg<uintT>::vector(P[1],P[2]).move_to(primitives);
        colors.insert(C); opacities.insert(O);
        CImg<uintT>::vector(P[2],P[3]).move_to(primitives);
        colors.insert(C); opacities.insert(O);
        CImg<uintT>::vector(P[3],P[0]).move_to(primitives);
        C.move_to(colors); O.move_to(opacities);
      } else {
        CImg<uintT>::vector(P[0]).move_to(primitives);
        colors.insert(C); opacities.insert(O);
        CImg<uintT>::vector(P[1]).move_to(primitives);
        colors.insert(C); opacities.insert(O);
        CImg<uintT>::vector(P[2]).move_to(primitives);
        colors.insert(C); opacities.insert(O);
        CImg<uintT>::vector(P[3]).move_to(primitives);
        C.move_to(colors); O.move_to(opacities);
      }
      break;
    case 5 : // Sphere.
      if (mode==2) {
        P.move_to(primitives); C.move_to(colors); O.move_to(opacities);
      } else if (mode==1) {
        CImg<uintT>::vector(P[0],P[1],1,P[3],P[4]).move_to(primitives);
        colors.insert(C); opacities.insert(O);
      } else {
        const float
          x0 = (float)(*this)(P[0],0),
          y0 = (float)(*this)(P[0],1),
          z0 = (float)(*this)(P[0],2),
          x1 = (float)(*this)(P[1],0),
          y1 = (float)(*this)(P[1],1),
          z1 = (float)(*this)(P[1],2);
        (*this)(P[0],0) = (T)((x0 + x1)/2);
        (*this)(P[0],1) = (T)((y0 + y1)/2);
        (*this)(P[0],2) = (T)((z0 + z1)/2);
        CImg<uintT>::vector(P[0]).move_to(primitives);
        colors.insert(C); opacities.insert(O);
      }
      break;
    case 6 : // Textured segment.
      if (mode==2) {
        CImg<uintT>::vector(P[0],P[1]).move_to(primitives);
        C.get_vector_at(P[2]>=C._width?C._width - 1:P[2],
                        P[3]>=C._height?C._height - 1:P[3]).move_to(colors);
        if (O.size()!=1) CImg<floatT>::vector(O._atXY(P[2],P[3])).move_to(opacities);
        else O.move_to(opacities);
      } else if (mode==1) {
        P.move_to(primitives); C.move_to(colors); O.move_to(opacities);
      } else {
        CImg<uintT>::vector(P[0]).move_to(primitives);
        C.get_vector_at(P[2]>=C._width?C._width - 1:P[2],
                        P[3]>=C._height?C._height - 1:P[3]).move_to(colors);
        if (O.size()!=1) CImg<floatT>::vector(O._atXY(P[2],P[3])).move_to(opacities);
        else opacities.insert(O);
        CImg<uintT>::vector(P[1]).move_to(primitives);
        C.get_vector_at(P[4]>=C._width?C._width - 1:P[4],
                        P[5]>=C._height?C._height - 1:P[5]).move_to(colors);
        if (O.size()!=1) CImg<floatT>::vector(O._atXY(P[4],P[5])).move_to(opacities);
        else O.move_to(opacities);
      }
      break;
    case 9 : // Textured triangle.
      if (mode==2) {
        CImg<uintT>::vector(P[0],P[1],P[2]).move_to(primitives);
        C.get_vector_at(P[3]>=C._width?C._width - 1:P[3],
                        P[4]>=C._height?C._height - 1:P[4]).move_to(colors);
        if (O.size()!=1) CImg<floatT>::vector(O._atXY(P[3],P[4])).move_to(opacities);
        else O.move_to(opacities);
      } else if (mode==1) {
        CImg<uintT>::vector(P[0],P[1],P[3],P[4],P[5],P[6]).
          move_to(primitives);
        C.move_to(colors); opacities.insert(O);
        CImg<uintT>::vector(P[1],P[2],P[3],P[4],P[7],P[8]).
          move_to(primitives);
        colors.insert(colors.back(),~0U,true); opacities.insert(O);
        CImg<uintT>::vector(P[2],P[0],P[7],P[8],P[1],P[3]).
          move_to(primitives);
        colors.insert(colors.back(),~0U,true); O.move_to(opacities);
      } else {
        CImg<uintT>::vector(P[0]).move_to(primitives);
        C.get_vector_at(P[3]>=C._width?C._width - 1:P[3],
                        P[4]>=C._height?C._height - 1:P[4]).move_to(colors);
        if (O.size()!=1) CImg<floatT>::vector(O._atXY(P[3],P[4])).move_to(opacities);
        else opacities.insert(O);
        CImg<uintT>::vector(P[1]).move_to(primitives);
        C.get_vector_at(P[5]>=C._width?C._width - 1:P[5],
                        P[6]>=C._height?C._height - 1:P[6]).move_to(colors);
        if (O.size()!=1) CImg<floatT>::vector(O._atXY(P[5],P[6])).move_to(opacities);
        else opacities.insert(O);
        CImg<uintT>::vector(P[2]).move_to(primitives);
        C.get_vector_at(P[7]>=C._width?C._width - 1:P[7],
                        P[8]>=C._height?C._height - 1:P[8]).move_to(colors);
        if (O.size()!=1) CImg<floatT>::vector(O._atXY(P[7],P[8])).move_to(opacities);
        else O.move_to(opacities);
      }
      break;
    case 12 : // Textured quadrangle.
      if (mode==2) {
        CImg<uintT>::vector(P[0],P[1],P[2],P[3]).move_to(primitives);
        C.get_vector_at(P[4]>=C._width?C._width - 1:P[4],
                        P[5]>=C._height?C._height - 1:P[5]).move_to(colors);
        if (O.size()!=1) CImg<floatT>::vector(O._atXY(P[4],P[5])).move_to(opacities);
        else O.move_to(opacities);
      } else if (mode==1) {
        CImg<uintT>::vector(P[0],P[1],P[4],P[5],P[6],P[7]).
          move_to(primitives);
        C.move_to(colors); opacities.insert(O);
        CImg<uintT>::vector(P[1],P[2],P[6],P[7],P[8],P[9]).
          move_to(primitives);
        colors.insert(colors.back(),~0U,true); opacities.insert(O);
        CImg<uintT>::vector(P[2],P[3],P[8],P[9],P[10],P[11]).
          move_to(primitives);
        colors.insert(colors.back(),~0U,true); opacities.insert(O);
        CImg<uintT>::vector(P[3],P[0],P[10],P[11],P[4],P[5]).
          move_to(primitives);
        colors.insert(colors.back(),~0U,true); O.move_to(opacities);
      } else {
        CImg<uintT>::vector(P[0]).move_to(primitives);
        C.get_vector_at(P[4]>=C._width?C._width - 1:P[4],
                        P[5]>=C._height?C._height - 1:P[5]).move_to(colors);
        if (O.size()!=1) CImg<floatT>::vector(O._atXY(P[4],P[5])).move_to(opacities);
        else opacities.insert(O);
        CImg<uintT>::vector(P[1]).move_to(primitives);
        C.get_vector_at(P[6]>=C._width?C._width - 1:P[6],
                        P[7]>=C._height?C._height - 1:P[7]).move_to(colors);
        if (O.size()!=1) CImg<floatT>::vector(O._atXY(P[6],P[7])).move_to(opacities);
        else opacities.insert(O);
        CImg<uintT>::vector(P[2]).move_to(primitives);
        C.get_vector_at(P[8]>=C._width?C._width - 1:P[8],
                        P[9]>=C._height?C._height - 1:P[9]).move_to(colors);
        if (O.size()!=1) CImg<floatT>::vector(O._atXY(P[8],P[9])).move_to(opacities);
        else opacities.insert(O);
        CImg<uintT>::vector(P[3]).move_to(primitives);
        C.get_vector_at(P[10]>=C._width?C._width - 1:P[10],
                        P[11]>=C._height?C._height - 1:P[11]).move_to(colors);
        if (O.size()!=1) CImg<floatT>::vector(O._atXY(P[10],P[11])).move_to(opacities);
        else O.move_to(opacities);
      }
      break;
    default : // Other primitives.
      P.move_to(primitives);
      C.move_to(colors);
      O.move_to(opacities);
    }
  }
  if (psiz) {
    primitives.remove(0,psiz - 1);
    colors.remove(0,psiz - 1);
    opacities.remove(0,psiz - 1);
  }
  object3dtoCImg3d(primitives,colors,opacities,false);
  return *this;
}

CImg<T> get_convert_primitives_CImg3d(const unsigned int mode) const {
  return (+*this).convert_primitives_CImg3d(mode);
}

// Additional filters.

CImg<T>& gmic_blur(const float sigma_x, const float sigma_y, const float sigma_z, const float sigma_c,
                   const bool boundary_conditions, const bool is_gaussian) {
  if (is_empty()) return *this;
  if (is_gaussian) {
    if (_width>1) vanvliet(sigma_x,0,'x',boundary_conditions);
    if (_height>1) vanvliet(sigma_y,0,'y',boundary_conditions);
    if (_depth>1) vanvliet(sigma_z,0,'z',boundary_conditions);
    if (_spectrum>1) vanvliet(sigma_c,0,'c',boundary_conditions);
  } else {
    if (_width>1) deriche(sigma_x,0,'x',boundary_conditions);
    if (_height>1) deriche(sigma_y,0,'y',boundary_conditions);
    if (_depth>1) deriche(sigma_z,0,'z',boundary_conditions);
    if (_spectrum>1) deriche(sigma_c,0,'c',boundary_conditions);
  }
  return *this;
}

CImg<Tfloat> get_gmic_blur(const float sigma_x, const float sigma_y, const float sigma_z, const float sigma_c,
                           const bool boundary_conditions, const bool is_gaussian) const {
  return CImg<Tfloat>(*this,false).gmic_blur(sigma_x,sigma_y,sigma_z,sigma_c,boundary_conditions,is_gaussian);
}

CImg<T>& gmic_blur_box(const float sigma_x, const float sigma_y, const float sigma_z, const float sigma_c,
                       const unsigned int order, const bool boundary_conditions) {
  if (is_empty()) return *this;
  if (_width>1) boxfilter(sigma_x,order,'x',boundary_conditions);
  if (_height>1) boxfilter(sigma_y,order,'y',boundary_conditions);
  if (_depth>1) boxfilter(sigma_z,order,'z',boundary_conditions);
  if (_spectrum>1) boxfilter(sigma_c,order,'c',boundary_conditions);
  return *this;
}

CImg<Tfloat> get_gmic_blur_box(const float sigma_x, const float sigma_y, const float sigma_z, const float sigma_c,
                               const unsigned int order, const bool boundary_conditions) const {
  return CImg<Tfloat>(*this,false).gmic_blur_box(sigma_x,sigma_y,sigma_z,sigma_c,order,boundary_conditions);
}

CImg<T>& gmic_blur_box(const float sigma, const unsigned int order, const bool boundary_conditions) {
  const float nsigma = sigma>=0?sigma:-sigma*cimg::max(_width,_height,_depth)/100;
  return gmic_blur_box(nsigma,nsigma,nsigma,0,order,boundary_conditions);
}

CImg<Tfloat> get_gmic_blur_box(const float sigma, const unsigned int order, const bool boundary_conditions) const {
  return CImg<Tfloat>(*this,false).gmic_blur_box(sigma,order,boundary_conditions);
}

template<typename t>
CImg<T>& inpaint(const CImg<t>& mask, const unsigned int method) {
  if (!is_sameXYZ(mask))
    throw CImgArgumentException("CImg<%s>::inpaint(): Invalid mask (%u,%u,%u,%u,%p) for "
                                "instance image (%u,%u,%u,%u,%p).",
                                pixel_type(),mask._width,mask._height,mask._depth,
                                mask._spectrum,mask._data,
                                _width,_height,_depth,_spectrum,_data);
  CImg<t> _mask(mask,false), _nmask(mask,false);
  bool is_pixel = false;

  do {
    is_pixel = false;

    if (depth()==1) { // 2d image.
      CImg_3x3(M,t);
      CImg_3x3(I,T);

      switch (method) {
      case 0: // Average 2d (low-connectivity).
        cimg_for3x3(_mask,x,y,0,0,M,t) if (Mcc && (!Mcp || !Mpc || !Mnc || !Mcn)) {
          is_pixel = true;
          const unsigned int wcp = Mcp?0U:1U, wpc = Mpc?0U:1U, wnc = Mnc?0U:1U, wcn = Mcn?0U:1U,
            sumw = wcp + wpc + wnc + wcn;
          cimg_forC(*this,k) {
            cimg_get3x3(*this,x,y,0,k,I,T);
            (*this)(x,y,k) = (T)((wcp*Icp + wpc*Ipc + wnc*Inc + wcn*Icn)/(float)sumw);
          }
          _nmask(x,y) = 0;
        }
        break;

      case 1: // Average 2d (high-connectivity).
        cimg_for3x3(_mask,x,y,0,0,M,t) if (Mcc && (!Mpp || !Mcp || !Mnp || !Mpc || !Mnc || !Mpn || !Mcn || !Mnn)) {
          is_pixel = true;
          const unsigned int
            wpp = Mpp?0U:1U, wcp = Mcp?0U:2U, wnp = Mnp?0U:1U,
            wpc = Mpc?0U:2U, wnc = Mnc?0U:2U,
            wpn = Mpn?0U:1U, wcn = Mcn?0U:2U, wnn = Mnn?0U:1U,
            sumw = wpp + wcp + wnp + wpc + wnc + wpn + wcn + wnn;
          cimg_forC(*this,k) {
            cimg_get3x3(*this,x,y,0,k,I,T);
            (*this)(x,y,k) = (T)((wpp*Ipp + wcp*Icp + wnp*Inp + wpc*Ipc +
                                  wnc*Inc + wpn*Ipn + wcn*Icn + wnn*Inn)/(float)sumw);
          }
          _nmask(x,y) = 0;
        }
        break;

      case 2: { // Median 2d (low-connectivity).
        T J[4];
        cimg_for3x3(_mask,x,y,0,0,M,t)
          if (Mcc && (!Mcp || !Mpc || !Mnc || !Mcn)) {
            is_pixel = true;
            cimg_forC(*this,k) {
              cimg_get3x3(*this,x,y,0,k,I,T);
              unsigned int ind = 0;
              if (!Mcp) J[ind++] = Icp;
              if (!Mpc) J[ind++] = Ipc;
              if (!Mnc) J[ind++] = Inc;
              if (!Mcn) J[ind++] = Icn;
              (*this)(x,y,k) = CImg<T>(J,ind,1,1,1,true).kth_smallest(ind>>1);
            }
            _nmask(x,y) = 0;
          }
      } break;

      default: // Median 2d (high-connectivity).
        T J[8];
        cimg_for3x3(_mask,x,y,0,0,M,t)
          if (Mcc && (!Mpp || !Mcp || !Mnp || !Mpc || !Mnc || !Mpn || !Mcn || !Mnn)) {
            is_pixel = true;
            cimg_forC(*this,k) {
              cimg_get3x3(*this,x,y,0,k,I,T);
              unsigned int ind = 0;
              if (!Mpp) J[ind++] = Ipp; if (!Mcp) J[ind++] = Icp; if (!Mnp) J[ind++] = Inp;
              if (!Mpc) J[ind++] = Ipc; if (!Mnc) J[ind++] = Inc;
              if (!Mpn) J[ind++] = Ipn; if (!Mcn) J[ind++] = Icn; if (!Mnn) J[ind++] = Inn;
              (*this)(x,y,k) = CImg<T>(J,ind,1,1,1,true).kth_smallest(ind>>1);
            }
            _nmask(x,y) = 0;
          }
      }

    } else { // 3d image.
      CImg_3x3x3(M,t);
      CImg_3x3x3(I,T);

      switch (method) {
      case 0: // Average 3d (low-connectivity).
        cimg_for3x3x3(_mask,x,y,z,0,M,t)
          if (Mccc && (!Mccp || !Mcpc || !Mpcc || !Mncc || !Mcnc || !Mccn)) {
            is_pixel = true;
            const unsigned int
              wccp = Mccp?0U:1U, wcpc = Mcpc?0U:1U, wpcc = Mpcc?0U:1U,
              wncc = Mncc?0U:1U, wcnc = Mcnc?0U:1U, wccn = Mccn?0U:1U,
              sumw = wcpc + wpcc + wccp + wncc + wcnc + wccn;
            cimg_forC(*this,k) {
              cimg_get3x3x3(*this,x,y,z,k,I,T);
              (*this)(x,y,z,k) = (T)((wccp*Iccp + wcpc*Icpc + wpcc*Ipcc +
                                      wncc*Incc + wcnc*Icnc + wccn*Iccn)/(float)sumw);
            }
            _nmask(x,y,z) = 0;
          }
        break;

      case 1: // Average 3d (high-connectivity).
        cimg_for3x3x3(_mask,x,y,z,0,M,t)
          if (Mccc && (!Mppp || !Mcpp || !Mnpp || !Mpcp || !Mccp || !Mncp || !Mpnp || !Mcnp ||
                       !Mnnp || !Mppc || !Mcpc || !Mnpc || !Mpcc || !Mncc || !Mpnc || !Mcnc ||
                       !Mnnc || !Mppn || !Mcpn || !Mnpn || !Mpcn || !Mccn || !Mncn || !Mpnn ||
                       !Mcnn || !Mnnn)) {
            is_pixel = true;
            const unsigned int
              wppp = Mppp?0U:1U, wcpp = Mcpp?0U:2U, wnpp = Mnpp?0U:1U,
              wpcp = Mpcp?0U:2U, wccp = Mccp?0U:4U, wncp = Mncp?0U:2U,
              wpnp = Mpnp?0U:1U, wcnp = Mcnp?0U:2U, wnnp = Mnnp?0U:1U,
              wppc = Mppc?0U:2U, wcpc = Mcpc?0U:4U, wnpc = Mnpc?0U:2U,
              wpcc = Mpcc?0U:4U, wncc = Mncc?0U:4U,
              wpnc = Mpnc?0U:2U, wcnc = Mcnc?0U:4U, wnnc = Mnnc?0U:2U,
              wppn = Mppn?0U:1U, wcpn = Mcpn?0U:2U, wnpn = Mnpn?0U:1U,
              wpcn = Mpcn?0U:2U, wccn = Mccn?0U:4U, wncn = Mncn?0U:2U,
              wpnn = Mpnn?0U:1U, wcnn = Mcnn?0U:2U, wnnn = Mnnn?0U:1U,
              sumw = wppp + wcpp + wnpp + wpcp + wccp + wncp + wpnp + wcnp + wnnp +
              wppc + wcpc + wnpc + wpcc + wncc + wpnc + wcnc + wnnc +
              wppn + wcpn + wnpn + wpcn + wccn + wncn + wpnn + wcnn + wnnn;
            cimg_forC(*this,k) {
              cimg_get3x3x3(*this,x,y,z,k,I,T);
              (*this)(x,y,z,k) = (T)((wppp*Ippp + wcpp*Icpp + wnpp*Inpp +
                                      wpcp*Ipcp + wccp*Iccp + wncp*Incp +
                                      wpnp*Ipnp + wcnp*Icnp + wnnp*Innp +
                                      wppc*Ippc + wcpc*Icpc + wnpc*Inpc +
                                      wpcc*Ipcc + wncc*Incc +
                                      wpnc*Ipnc + wcnc*Icnc + wnnc*Innc +
                                      wppn*Ippn + wcpn*Icpn + wnpn*Inpn +
                                      wpcn*Ipcn + wccn*Iccn + wncn*Incn +
                                      wpnn*Ipnn + wcnn*Icnn + wnnn*Innn)/(float)sumw);
            }
            _nmask(x,y,z) = 0;
          }
        break;

      case 2: { // Median 3d (low-connectivity).
        T J[6];
        cimg_for3x3x3(_mask,x,y,z,0,M,t)
          if (Mccc && (!Mccp || !Mcpc || !Mpcc || !Mncc || !Mcnc || !Mccn)) {
            is_pixel = true;
            cimg_forC(*this,k) {
              cimg_get3x3x3(*this,x,y,z,k,I,T);
              unsigned int ind = 0;
              if (!Mccp) J[ind++] = Iccp; if (!Mcpc) J[ind++] = Icpc; if (!Mpcc) J[ind++] = Ipcc;
              if (!Mncc) J[ind++] = Incc; if (!Mcnc) J[ind++] = Icnc; if (!Mccn) J[ind++] = Iccn;
              (*this)(x,y,z,k) = CImg<T>(J,ind,1,1,1,true).kth_smallest(ind>>1);
            }
            _nmask(x,y,z) = 0;
          }
      } break;

      default: { // Median 3d (high-connectivity).
        T J[26];
        cimg_for3x3x3(_mask,x,y,z,0,M,t)
          if (Mccc && (!Mppp || !Mcpp || !Mnpp || !Mpcp || !Mccp || !Mncp || !Mpnp || !Mcnp ||
                       !Mnnp || !Mppc || !Mcpc || !Mnpc || !Mpcc || !Mncc || !Mpnc || !Mcnc ||
                       !Mnnc || !Mppn || !Mcpn || !Mnpn || !Mpcn || !Mccn || !Mncn || !Mpnn ||
                       !Mcnn || !Mnnn)) {
            is_pixel = true;
            cimg_forC(*this,k) {
              cimg_get3x3x3(*this,x,y,z,k,I,T);
              unsigned int ind = 0;
              if (!Mppp) J[ind++] = Ippp; if (!Mcpp) J[ind++] = Icpp; if (!Mnpp) J[ind++] = Inpp;
              if (!Mpcp) J[ind++] = Ipcp; if (!Mccp) J[ind++] = Iccp; if (!Mncp) J[ind++] = Incp;
              if (!Mpnp) J[ind++] = Ipnp; if (!Mcnp) J[ind++] = Icnp; if (!Mnnp) J[ind++] = Innp;
              if (!Mppc) J[ind++] = Ippc; if (!Mcpc) J[ind++] = Icpc; if (!Mnpc) J[ind++] = Inpc;
              if (!Mpcc) J[ind++] = Ipcc; if (!Mncc) J[ind++] = Incc;
              if (!Mpnc) J[ind++] = Ipnc; if (!Mcnc) J[ind++] = Icnc; if (!Mnnc) J[ind++] = Innc;
              if (!Mppn) J[ind++] = Ippn; if (!Mcpn) J[ind++] = Icpn; if (!Mnpn) J[ind++] = Inpn;
              if (!Mpcn) J[ind++] = Ipcn; if (!Mccn) J[ind++] = Iccn; if (!Mncn) J[ind++] = Incn;
              if (!Mpnn) J[ind++] = Ipnn; if (!Mcnn) J[ind++] = Icnn; if (!Mnnn) J[ind++] = Innn;
              (*this)(x,y,z,k) = CImg<T>(J,ind,1,1,1,true).kth_smallest(ind>>1);
            }
            _nmask(x,y,z) = 0;
          }
      } break;
      }
    }

    _mask = _nmask;
  } while (is_pixel);
  return *this;
}

template<typename t>
CImg<T> get_inpaint(const CImg<t>& mask, const unsigned int method) const {
  return (+*this).inpaint(mask,method);
}

template<typename t>
CImg<T>& inpaint_patch(const CImg<t>& mask, const unsigned int patch_size=11,
                       const unsigned int lookup_size=22, const float lookup_factor=1,
                       const int lookup_increment=1,
                       const unsigned int blend_size=0, const float blend_threshold=0.5f,
                       const float blend_decay=0.02, const unsigned int blend_scales=10,
                       const bool is_blend_outer=false) {
  if (depth()>1)
    throw CImgInstanceException(_cimg_instance
                                "inpaint_patch(): Instance image is volumetric (should be 2d).",
                                cimg_instance);
  if (!is_sameXYZ(mask))
    throw CImgArgumentException(_cimg_instance
                                "inpaint_patch() : Sizes of instance image and specified mask "
                                "(%u,%u,%u,%u) do not match.",
                                cimg_instance,
                                mask._width,mask._height,mask._depth,mask._spectrum);
  if (!patch_size)
    throw CImgArgumentException(_cimg_instance
                                "inpaint_patch() : Specified patch size is 0, must be strictly "
                                "positive.",
                                cimg_instance);
  if (!lookup_size)
    throw CImgArgumentException(_cimg_instance
                                "inpaint_patch() : Specified lookup size is 0, must be strictly "
                                "positive.",
                                cimg_instance);
  if (lookup_factor<0)
    throw CImgArgumentException(_cimg_instance
                                "inpaint_patch() : Specified lookup factor %g is negative, must be "
                                "positive.",
                                cimg_instance,
                                lookup_factor);
  if (!lookup_increment)
    throw CImgArgumentException(_cimg_instance
                                "inpaint_patch() : Specified lookup increment is 0, must be "
                                "strictly positive.",
                                cimg_instance);
  if (blend_decay<0)
    throw CImgArgumentException(_cimg_instance
                                "inpaint_patch() : Specified blend decay %g is negative, must be "
                                "positive.",
                                cimg_instance,
                                blend_decay);

  // Find (dilated by 2) bounding box for the inpainting mask.
  unsigned int xm0 = _width, ym0 = _height, xm1 = 0, ym1 = 0;
  bool is_mask_found = false;
  cimg_forXY(mask,x,y) if (mask(x,y)) {
    is_mask_found = true;
    if (x<(int)xm0) xm0 = (unsigned int)x;
    if (x>(int)xm1) xm1 = (unsigned int)x;
    if (y<(int)ym0) ym0 = (unsigned int)y;
    if (y>(int)ym1) ym1 = (unsigned int)y;
  }
  if (!is_mask_found) return *this;
  xm0 = xm0>2?xm0 - 2:0;
  ym0 = ym0>2?ym0 - 2:0;
  xm1 = xm1<_width - 3?xm1 + 2:_width - 1;
  ym1 = ym1<_height - 3?ym1 + 2:_height - 1;
  int ox = (int)xm0, oy = (int)ym0;
  unsigned int dx = xm1 - xm0 + 1U, dy = ym1 - ym0 + 1U;

  // Construct normalized version of the mask.
  CImg<ucharT> nmask(dx,dy);
  unsigned char *ptrM = nmask.data();
  cimg_for_inXY(mask,xm0,ym0,xm1,ym1,x,y) *(ptrM++) = mask(x,y)?0U:1U;
  xm0 = ym0 = 0; xm1 = dx - 1; ym1 = dy - 1;

  // Start patch filling algorithm.
  const int p2 = (int)patch_size/2, p1 = (int)patch_size - p2 - 1;
  const unsigned int patch_size2 = patch_size*patch_size;
  unsigned int _lookup_size = lookup_size, nb_lookups = 0, nb_fails = 0, nb_saved_patches = 0;
  bool is_strict_search = true;
  const float one = 1;

  CImg<floatT> confidences(nmask), priorities(dx,dy,1,2,-1), pC;
  CImg<unsigned int> saved_patches(4,256), is_visited(width(),height(),1,1,0);
  CImg<ucharT> pM, pN;  // Pre-declare patch variables (avoid iterative memory alloc/dealloc).
  CImg<T> pP, pbest;
  CImg<floatT> weights(patch_size,patch_size,1,1,0);
  weights.draw_gaussian((float)p1,(float)p1,patch_size/15.0f,&one)/=patch_size2;
  unsigned int target_index = 0;

  while (true) {

    // Extract mask border points and compute priorities to find target point.
    unsigned int nb_border_points = 0;
    float target_confidence = -1, target_priority = -1;
    int target_x = -1, target_y = -1;
    CImg_5x5(M,unsigned char);

    cimg_for_in5x5(nmask,xm0,ym0,xm1,ym1,x,y,0,0,M,unsigned char)
      if (!Mcc && (Mcp || Mcn || Mpc || Mnc)) { // Found mask border point.

        float confidence_term = -1, data_term = -1;
        if (priorities(x,y)>=0) { // If priority has already been computed.
          confidence_term = priorities(x,y,0);
          data_term = priorities(x,y,1);
        } else { // If priority must be computed/updated.

          // Compute smoothed normal vector.
          const float
            // N = smoothed 3x3 neighborhood of M.
            Npc = (4.0f*Mpc + 2.0f*Mbc + 2.0f*Mcc + 2.0f*Mpp + 2.0f*Mpn + Mbp + Mbn + Mcp + Mcn)/16,
            Nnc = (4.0f*Mnc + 2.0f*Mac + 2.0f*Mcc + 2.0f*Mnp + 2.0f*Mnn + Map + Man + Mcp + Mcn)/16,
            Ncp = (4.0f*Mcp + 2.0f*Mcb + 2.0f*Mcc + 2.0f*Mpp + 2.0f*Mnp + Mpb + Mnb + Mpc + Mnc)/16,
            Ncn = (4.0f*Mcn + 2.0f*Mca + 2.0f*Mcc + 2.0f*Mpn + 2.0f*Mnn + Mpa + Mna + Mpc + Mnc)/16,
            _nx = 0.5f*(Nnc - Npc),
            _ny = 0.5f*(Ncn - Ncp),
            nn = std::sqrt(1e-8f + _nx*_nx + _ny*_ny),
            nx = _nx/nn,
            ny = _ny/nn;

          // Compute confidence term.
          nmask._inpaint_patch_crop(x - p1,y - p1,x + p2,y + p2,1).move_to(pM);
          confidences._inpaint_patch_crop(x - p1,y - p1,x + p2,y + p2,1).move_to(pC);
          confidence_term = 0;
          const unsigned char *ptrM = pM.data();
          cimg_for(pC,ptrC,float) confidence_term+=*ptrC**(ptrM++);
          confidence_term/=patch_size2;
          priorities(x,y,0) = confidence_term;

          // Compute data term.
          _inpaint_patch_crop(ox + x - p1,oy + y - p1,ox + x + p2,oy + y + p2,2).move_to(pP);
          float mean_ix2 = 0, mean_ixiy = 0, mean_iy2 = 0;

          CImg_3x3(I,T);
          CImg_3x3(_M, unsigned char);
          cimg_forC(pP,c) cimg_for3x3(pP,p,q,0,c,I,T) {
            // Compute weight-mean of structure tensor inside patch.
            cimg_get3x3(pM,p,q,0,0,_M,unsigned char);
            const float
              ixf = (float)(_Mnc*_Mcc*(Inc - Icc)),
              iyf = (float)(_Mcn*_Mcc*(Icn - Icc)),
              ixb = (float)(_Mcc*_Mpc*(Icc - Ipc)),
              iyb = (float)(_Mcc*_Mcp*(Icc - Icp)),
              ix = cimg::abs(ixf)>cimg::abs(ixb)?ixf:ixb,
              iy = cimg::abs(iyf)>cimg::abs(iyb)?iyf:iyb,
              w = weights(p,q);
            mean_ix2 += w*ix*ix;
            mean_ixiy += w*ix*iy;
            mean_iy2 += w*iy*iy;
          }
          const float // Compute tensor-directed data term.
            ux = mean_ix2*(-ny) + mean_ixiy*nx,
            uy = mean_ixiy*(-ny) + mean_iy2*nx;
          data_term = std::sqrt(ux*ux + uy*uy);
          priorities(x,y,1) = data_term;
        }
        const float priority = confidence_term*data_term;
        if (priority>target_priority) {
          target_priority = priority; target_confidence = confidence_term;
          target_x = ox + x; target_y = oy + y;
        }
        ++nb_border_points;
      }
    if (!nb_border_points) break; // No more mask border points to inpaint!

    // Locate already reconstructed neighbors (if any), to get good origins for patch lookup.
    CImg<unsigned int> lookup_candidates(2,256);
    unsigned int nb_lookup_candidates = 0, *ptr_lookup_candidates = lookup_candidates.data();
    const unsigned int *ptr_saved_patches = saved_patches.data();
    const int
      x0 = target_x - (int)patch_size, y0 = target_y - (int)patch_size,
      x1 = target_x + (int)patch_size, y1 = target_y + (int)patch_size;
    for (unsigned int k = 0; k<nb_saved_patches; ++k) {
      const unsigned int
        src_x = *(ptr_saved_patches++), src_y = *(ptr_saved_patches++),
        dest_x = *(ptr_saved_patches++), dest_y = *(ptr_saved_patches++);
      if ((int)dest_x>=x0 && (int)dest_y>=y0 && (int)dest_x<=x1 && (int)dest_y<=y1) {
        const int off_x = target_x - (int)dest_x, off_y = target_y - (int)dest_y;
        *(ptr_lookup_candidates++) = src_x + off_x;
        *(ptr_lookup_candidates++) = src_y + off_y;
        if (++nb_lookup_candidates>=lookup_candidates._height) {
          lookup_candidates.resize(2,-200,1,1,0);
          ptr_lookup_candidates = lookup_candidates.data(0,nb_lookup_candidates);
        }
      }
    }
    // Add also target point as a center for the patch lookup.
    if (++nb_lookup_candidates>=lookup_candidates._height) {
      lookup_candidates.resize(2,-200,1,1,0);
      ptr_lookup_candidates = lookup_candidates.data(0,nb_lookup_candidates);
    }
    *(ptr_lookup_candidates++) = (unsigned int)target_x;
    *(ptr_lookup_candidates++) = (unsigned int)target_y;

    // Divide size of lookup regions if several lookup sources have been detected.
    unsigned int final_lookup_size = _lookup_size;
    if (nb_lookup_candidates>1) {
      const unsigned int
        _final_lookup_size = cimg::max(5U,(unsigned int)cimg::round(_lookup_size*lookup_factor/
                                                                    std::sqrt((float)nb_lookup_candidates),1,1));
      final_lookup_size = _final_lookup_size + 1 - (_final_lookup_size%2);
    }
    const int l2 = (int)final_lookup_size/2, l1 = (int)final_lookup_size - l2 - 1;

#ifdef gmic_debug
    CImg<ucharT> visu(*this,false);
    for (unsigned int C = 0; C<nb_lookup_candidates; ++C) {
      const int
        xl = lookup_candidates(0,C),
        yl = lookup_candidates(1,C);
      visu.draw_rectangle(xl - l1,yl - l1,xl + l2,yl + l2,CImg<ucharT>::vector(0,255,0).data(),0.2f);
    }
    visu.draw_rectangle(target_x - p1,target_y - p1,target_x + p2,target_y + p2,
                        CImg<ucharT>::vector(255,0,0).data(),0.5f);
    static int foo = 0;
    if (!(foo%1)) {
      static CImgDisplay disp_debug;
      disp_debug.display(visu).set_title("DEBUG");
    }
    ++foo;
#endif // #ifdef gmic_debug

    // Find best patch candidate to fill target point.
    _inpaint_patch_crop(target_x - p1,target_y - p1,target_x + p2,target_y + p2,0).move_to(pP);
    nmask._inpaint_patch_crop(target_x - ox - p1,target_y - oy - p1,target_x - ox + p2,target_y - oy + p2,0).
      move_to(pM);
    ++target_index;
    const unsigned int
      _lookup_increment = (unsigned int)(lookup_increment>0?lookup_increment:
                                         nb_lookup_candidates>1?1:-lookup_increment);
    float best_ssd = cimg::type<float>::max();
    int best_x = -1, best_y = -1;
    for (unsigned int C = 0; C<nb_lookup_candidates; ++C) {
      const int
        xl = (int)lookup_candidates(0,C),
        yl = (int)lookup_candidates(1,C),
        x0 = cimg::max(p1,xl - l1), y0 = cimg::max(p1,yl - l1),
        x1 = cimg::min(width() - 1 - p2,xl + l2), y1 = cimg::min(height() - 1 - p2,yl + l2);
      for (int y = y0; y<=y1; y+=_lookup_increment)
        for (int x = x0; x<=x1; x+=_lookup_increment) if (is_visited(x,y)!=target_index) {
            if (is_strict_search) mask._inpaint_patch_crop(x - p1,y - p1,x + p2,y + p2,1).move_to(pN);
            else nmask._inpaint_patch_crop(x - ox - p1,y - oy - p1,x - ox + p2,y - oy + p2,0).move_to(pN);
            if ((is_strict_search && pN.sum()==0) || (!is_strict_search && pN.sum()==patch_size2)) {
              _inpaint_patch_crop(x - p1,y - p1,x + p2,y + p2,0).move_to(pC);
              float ssd = 0;
              const T *_pP = pP._data;
              const float *_pC = pC._data;
              cimg_for(pM,_pM,unsigned char) { if (*_pM) {
                  cimg_forC(pC,c) {
                    ssd+=cimg::sqr((Tfloat)*_pC - (Tfloat)*_pP); _pC+=patch_size2; _pP+=patch_size2;
                  }
                  if (ssd>=best_ssd) break;
                  _pC-=pC._spectrum*patch_size2;
                  _pP-=pC._spectrum*patch_size2;
                }
                ++_pC; ++_pP;
              }
              if (ssd<best_ssd) { best_ssd = ssd; best_x = x; best_y = y; }
            }
            is_visited(x,y) = target_index;
          }
    }

    if (best_x<0) { // If no best patch found.
      priorities(target_x - ox,target_y - oy,0)/=10; // Reduce its priority (lower data_term).
      if (++nb_fails>=4) { // If too much consecutive fails :
        nb_fails = 0;
        _lookup_size+=_lookup_size/2; // Try to expand the lookup size.
        if (++nb_lookups>=3) {
          if (is_strict_search) { // If still fails, switch to non-strict search mode.
            is_strict_search = false;
            _lookup_size = lookup_size;
            nb_lookups = 0;
          }
          else return *this; // Pathological case, probably a weird mask.
        }
      }
    } else { // Best patch found -> reconstruct missing part on the target patch.
      _lookup_size = lookup_size;
      nb_lookups = nb_fails = 0;
      _inpaint_patch_crop(best_x - p1,best_y - p1,best_x + p2,best_y + p2,0).move_to(pbest);
      nmask._inpaint_patch_crop(target_x - ox - p1,target_y - oy - p1,target_x - ox + p2,target_y - oy + p2,1).
        move_to(pM);
      cimg_for(pM,ptr,unsigned char) *ptr = (unsigned char)(1 - *ptr);
      draw_image(target_x - p1,target_y - p1,pbest,pM,1,1);
      confidences.draw_image(target_x - ox - p1,target_y - oy - p1,pC.fill(target_confidence),pM,1,1);
      nmask.draw_rectangle(target_x - ox - p1,target_y - oy - p1,0,0,target_x - ox + p2,target_y - oy + p2,0,0,1);
      priorities.draw_rectangle(target_x - ox - (int)patch_size,
                                target_y - oy - (int)patch_size,0,0,
                                target_x - ox + 3*p2/2,
                                target_y - oy + 3*p2/2,0,0,-1);
      // Remember patch positions.
      unsigned int *ptr_saved_patches = saved_patches.data(0,nb_saved_patches);
      *(ptr_saved_patches++) = (unsigned int)best_x;
      *(ptr_saved_patches++) = (unsigned int)best_y;
      *(ptr_saved_patches++) = (unsigned int)target_x;
      *ptr_saved_patches = (unsigned int)target_y;
      if (++nb_saved_patches>=saved_patches._height) saved_patches.resize(4,-200,1,1,0);
    }
  }
  nmask.assign();  // Free some unused memory resources.
  priorities.assign();
  confidences.assign();
  is_visited.assign();

  // Blend inpainting result (if requested), using multi-scale blending algorithm.
  if (blend_size && blend_scales) {
    const float _blend_threshold = cimg::max(0.0f,cimg::min(1.0f,blend_threshold));
    saved_patches._height = nb_saved_patches;

    // Re-crop image and mask if outer blending is activated.
    if (is_blend_outer) {
      const int
        b2 = (int)blend_size/2, b1 = (int)blend_size - b2 - 1,
        xb0 = cimg::max(0,ox - b1),
        yb0 = cimg::max(0,oy - b1),
        xb1 = (int)cimg::min(_width - 1,xb0 + dx + b1 + b2),
        yb1 = (int)cimg::min(_height - 1,yb0 + dy + b1 + b2);
      ox = xb0; oy = yb0; dx = xb1 - xb0 + 1U, dy = yb1 - yb0 + 1U;
    }

    // Generate map of source offsets.
    CImg<unsigned int> offsets(dx,dy,1,2);
    unsigned int *ptr = saved_patches.end();
    cimg_forY(saved_patches,i) {
      const unsigned int yd = *(--ptr), xd = *(--ptr), ys = *(--ptr), xs = *(--ptr);
      for (int l = -p1; l<=p2; ++l)
        for (int k = -p1; k<=p2; ++k) {
          const int xdk = (int)xd + k, ydl = (int)yd + l;
          if (xdk>=0 && xdk<=width() - 1 && ydl>=0 && ydl<=height() - 1 && mask(xd + k,yd + l)) {
            offsets(xd - ox + k,yd - oy + l,0) = xs + k;
            offsets(xd - ox + k,yd - oy + l,1) = ys + l;
          }
        }
    }
    unsigned int *ptrx = offsets.data(0,0,0,0), *ptry = offsets.data(0,0,0,1);
    cimg_forXY(offsets,x,y) {
      if (!mask(x + ox,y + oy)) { *ptrx = (unsigned int)(x + ox); *ptry = (unsigned int)(y + oy); }
      ++ptrx; ++ptry;
    }

    // Generate map of local blending amplitudes.
    CImg<floatT> blend_map(dx,dy,1,1,0);
    CImg_3x3(I,float);
    cimg_for3XY(offsets,x,y) if (mask(x + ox,y + oy)) {
      const float
        iox = cimg::max((float)offsets(_n1x,y,0) - offsets(x,y,0),
                        (float)offsets(x,y,0) - offsets(_p1x,y,0)),
        ioy = cimg::max((float)offsets(x,_n1y,1) - offsets(x,y,1),
                        (float)offsets(x,y,1) - offsets(x,_p1y,1)),
        ion = std::sqrt(iox*iox + ioy*ioy);
      float iin = 0;
      cimg_forC(*this,c) {
        cimg_get3x3(*this,x,y,0,c,I,float);
        const float
          iix = (float)cimg::max(Inc - Icc,Icc - Ipc),
          iiy = (float)cimg::max(Icn - Icc,Icc - Icp);
        iin+=std::log(1 + iix*iix + iiy*iiy);
      }
      iin/=_spectrum;
      blend_map(x,y) = ion*iin;
    }
    blend_map.threshold(blend_map.max()*_blend_threshold).distance(1);
    cimg_forXY(blend_map,x,y) blend_map(x,y) = 1/(1 + blend_decay*blend_map(x,y));
    blend_map.quantize(blend_scales + 1,false);
    float bm, bM = blend_map.max_min(bm);
    if (bm==bM) blend_map.fill((float)blend_scales);

    // Generate blending scales.
    CImg<T> result = _inpaint_patch_crop(ox,oy,ox + dx - 1,oy + dy - 1,0);
    for (unsigned int blend_iter = 1; blend_iter<=blend_scales; ++blend_iter) {
      const unsigned int
        _blend_width = blend_iter*blend_size/blend_scales,
        blend_width = _blend_width?_blend_width + 1 - (_blend_width%2):0;
      if (!blend_width) continue;
      const int b2 = (int)blend_width/2, b1 = (int)blend_width - b2 - 1;
      CImg<floatT>
        blended = _inpaint_patch_crop(ox,oy,ox + dx - 1,oy + dy - 1,0),
        cumul(dx,dy,1,1);
      weights.assign(blend_width,blend_width,1,1,0).
        draw_gaussian((float)b1,(float)b1,blend_width/4.0f,&one);
      cimg_forXY(cumul,x,y) cumul(x,y) = mask(x + ox,y + oy)?0.0f:1.0f;
      blended.mul(cumul);

      cimg_forY(saved_patches,l) {
        const unsigned int *ptr = saved_patches.data(0,l);
        const int
          xs = (int)*(ptr++),
          ys = (int)*(ptr++),
          xd = (int)*(ptr++),
          yd = (int)*(ptr++);
        if (xs - b1<0 || ys - b1<0 || xs + b2>=width() || ys + b2>=height()) { // Blend with partial patch.
          const int
            xs0 = cimg::max(0,xs - b1),
            ys0 = cimg::max(0,ys - b1),
            xs1 = cimg::min(width() - 1,xs + b2),
            ys1 = cimg::min(height() - 1,ys + b2);
          _inpaint_patch_crop(xs0,ys0,xs1,ys1,0).move_to(pP);
          weights._inpaint_patch_crop(xs0 - xs + b1,ys0 - ys + b1,xs1 - xs + b1,ys1 - ys + b1,0).move_to(pC);
          blended.draw_image(xd + xs0 - xs - ox,yd + ys0 - ys - oy,pP,pC,-1);
          cumul.draw_image(xd + xs0 - xs - ox,yd + ys0 - ys - oy,pC,-1);
        } else { // Blend with full-size patch.
          _inpaint_patch_crop(xs - b1,ys - b1,xs + b2,ys + b2,0).move_to(pP);
          blended.draw_image(xd - b1 - ox,yd - b1 - oy,pP,weights,-1);
          cumul.draw_image(xd - b1 - ox,yd - b1 - oy,weights,-1);
        }
      }

      if (is_blend_outer) {
        cimg_forXY(blended,x,y) if (blend_map(x,y)==blend_iter) {
          const float cum = cumul(x,y);
          if (cum>0) cimg_forC(*this,c) result(x,y,c) = (T)(blended(x,y,c)/cum);
        }
      } else { cimg_forXY(blended,x,y) if (mask(x + ox,y + oy) && blend_map(x,y)==blend_iter) {
          const float cum = cumul(x,y);
          if (cum>0) cimg_forC(*this,c) result(x,y,c) = (T)(blended(x,y,c)/cum);
        }
      }
    }
    if (is_blend_outer) draw_image(ox,oy,result);
    else cimg_forXY(result,x,y) if (mask(x + ox,y + oy))
           cimg_forC(*this,c) (*this)(x + ox,y + oy,c) = (T)result(x,y,c);
  }
  return *this;
}

// Special crop function that supports more boundary conditions :
// 0=dirichlet (with value 0), 1=dirichlet (with value 1) and 2=neumann.
CImg<T> _inpaint_patch_crop(const int x0, const int y0, const int x1, const int y1,
                            const unsigned int boundary=0) const {
  const int
    nx0 = x0<x1?x0:x1, nx1 = x0^x1^nx0,
    ny0 = y0<y1?y0:y1, ny1 = y0^y1^ny0;
  CImg<T> res(1U + nx1 - nx0,1U + ny1 - ny0,1,_spectrum);
  if (nx0<0 || nx1>=width() || ny0<0 || ny1>=height()) {
    if (boundary>=2) cimg_forXYZC(res,x,y,z,c) res(x,y,z,c) = _atXY(nx0 + x,ny0 + y,z,c);
    else res.fill((T)boundary).draw_image(-nx0,-ny0,*this);
  } else res.draw_image(-nx0,-ny0,*this);
  return res;
}

template<typename t>
CImg<T> get_inpaint_patch(const CImg<t>& mask, const unsigned int patch_size=11,
                          const unsigned int lookup_size=22, const float lookup_factor=1,
                          const int lookup_increment=1,
                          const unsigned int blend_size=0, const float blend_threshold=0.5,
                          const float blend_decay=0.02f, const unsigned int blend_scales=10,
                          const bool is_blend_outer=false) const {
  return (+*this).inpaint_patch(mask,patch_size,lookup_size,lookup_factor,lookup_increment,
                                blend_size,blend_threshold,blend_decay,blend_scales,is_blend_outer);
}

CImg<T>& gmic_patchmatch(const CImg<T>& patch_image,
                         const unsigned int patch_width,
                         const unsigned int patch_height,
                         const unsigned int patch_depth=1,
                         const unsigned int nb_iterations=5,
                         const unsigned int nb_randoms=5,
                         const bool is_score=false,
                         const CImg<T> *const initialization=0) {
  return get_gmic_patchmatch(patch_image,patch_width,patch_height,patch_depth,
                             nb_iterations,nb_randoms,is_score,initialization).move_to(*this);
}

CImg<T> get_gmic_patchmatch(const CImg<T>& patch_image,
                            const unsigned int patch_width,
                            const unsigned int patch_height,
                            const unsigned int patch_depth=1,
                            const unsigned int nb_iterations=5,
                            const unsigned int nb_randoms=5,
                            const bool is_score=false,
                            const CImg<T> *const initialization=0) const {
  CImg<floatT> score, res;
  res = _get_patchmatch(patch_image,patch_width,patch_height,patch_depth,
                        nb_iterations,nb_randoms,
                        initialization?*initialization:CImg<T>::const_empty(),
                        is_score,is_score?score:CImg<floatT>::empty());
  if (score) res.resize(-100,-100,-100,3,0).draw_image(0,0,0,2,score);
  return res;
}

// Additional convenience plug-in functions.
CImg<T>& copymark() {
  return get_copymark().move_to(*this);
}

CImg<T> get_copymark() const {
  if (is_empty()) return CImg<T>::string("~");
  CImg<T> res = get_resize(_width + 1,1,1,1,0);
  const char *const ext = cimg::split_filename(_data);
  if (*ext) {
    const int l = (int)(ext - _data - 1);
    if (l>0) {
      if (_data[l - 1]=='~') return +*this;
      std::memcpy(res._data,_data,l);
    }
    res[l] = '~'; res[l + 1] = '.';
    std::memcpy(res._data + l + 2,ext,_data + _width - ext);
  } else {
    const int l = (int)(ext - _data);
    if (_data[l - 1]=='~' || (l>1 && _data[l - 1]==']' && _data[l - 2]=='~')) return +*this;
    std::memcpy(res._data,_data,l);
    res[l] = '~';
    if (ext>_data && *(ext - 1)==']') cimg::swap(res[l],res[l - 1]);
    std::memcpy(res._data + l + 1,ext,_data + _width - ext);
  }
  return res;
}

const CImg<T>& gmic_print(const char *const title, const bool is_debug,
                          const bool is_valid) const {
  cimg::mutex(29);
  CImg<doubleT> st;
  if (is_valid && !is_empty()) st = get_stats();
  const unsigned long siz = size(), msiz = siz*sizeof(T), siz1 = siz - 1,
    mdisp = msiz<8*1024?0U:msiz<8*1024*1024?1U:2U,
    wh = _width*_height, whd = _width*_height*_depth,
    w1 = _width - 1, wh1 = _width*_height - 1, whd1 = _width*_height*_depth - 1;

  std::fprintf(cimg::output(),"%s%s%s%s:\n  %ssize%s = (%u,%u,%u,%u) [%lu %s of %s%ss].\n  %sdata%s = %s",
               cimg::t_magenta,cimg::t_bold,title,cimg::t_normal,
               cimg::t_bold,cimg::t_normal,_width,_height,_depth,_spectrum,
               mdisp==0?msiz:(mdisp==1?(msiz>>10):(msiz>>20)),
               mdisp==0?"b":(mdisp==1?"Kio":"Mio"),
               _is_shared?"shared ":"",
               cimg::type<T>::string(),
               cimg::t_bold,cimg::t_normal,
               is_debug?"":"(");
  if (is_debug) std::fprintf(cimg::output(),"%p = (",(void*)_data);

  if (is_valid) {
    if (is_empty()) std::fprintf(cimg::output(),") [%s].\n",
                                 pixel_type());
    else {
      cimg_foroff(*this,off) {
        std::fprintf(cimg::output(),cimg::type<T>::format(),cimg::type<T>::format(_data[off]));
        if (off!=siz1) std::fprintf(cimg::output(),"%s",
                                    off%whd==whd1?"^":
                                    off%wh==wh1?"\\":
                                    off%_width==w1?";":",");
        if (off==11 && siz>24) { off = siz1 - 12; std::fprintf(cimg::output(),"(...),"); }
      }
      std::fprintf(cimg::output(),")%s.\n  %smin%s = %g, %smax%s = %g, %smean%s = %g, "
                   "%sstd%s = %g, %scoords_min%s = (%u,%u,%u,%u), "
                   "%scoords_max%s = (%u,%u,%u,%u).\n",
                   _is_shared?" [shared]":"",
                   cimg::t_bold,cimg::t_normal,st[0],
                   cimg::t_bold,cimg::t_normal,st[1],
                   cimg::t_bold,cimg::t_normal,st[2],
                   cimg::t_bold,cimg::t_normal,std::sqrt(st[3]),
                   cimg::t_bold,cimg::t_normal,(int)st[4],(int)st[5],(int)st[6],(int)st[7],
                   cimg::t_bold,cimg::t_normal,(int)st[8],(int)st[9],(int)st[10],(int)st[11]);
    }
  } else std::fprintf(cimg::output(),"%s%sinvalid pointer%s) [shared %s].\n",
                      cimg::t_red,cimg::t_bold,cimg::t_normal,
                      pixel_type());
  std::fflush(cimg::output());
  cimg::mutex(29,0);
  return *this;
}

//--------------- End of CImg plug-in ----------------------------

#else // #ifdef cimg_plugin

#include "gmic.h"
#include "gmic_stdlib.h"
using namespace cimg_library;
#undef min
#undef max

// Define convenience macros, variables and functions.
//----------------------------------------------------

// Define default pixel type.
#ifndef gmic_pixel_type
#define gmic_pixel_type float
#endif

// Macro to force stringifying selection for error messages.
#define gmic_selection_err selection2string(selection,images_names,1,gmic_selection)

// Return image argument as a shared or non-shared copy of one existing image.
inline bool _gmic_image_arg(const unsigned int ind, const CImg<unsigned int>& selection) {
  cimg_forY(selection,l) if (selection[l]==ind) return true;
  return false;
}
#define gmic_image_arg(ind) gmic_check(_gmic_image_arg(ind,selection)?images[ind]:\
                                       images[ind].get_shared())

// Macro to manage argument substitutions from a command.
template<typename T>
void gmic::_gmic_substitute_args(const char *const argument, const char *const argument0,
                                 const char *const command, const CImgList<T>& images) {
  if (is_debug) {
    if (std::strcmp(argument,argument0)) debug(images,"Command '%s': arguments = '%s' -> '%s'.",
                                               command,argument0,argument); \
    else debug(images,"Command '%s': arguments = '%s'.",
               command,argument0);
  }
}

#define gmic_substitute_args() { \
  const char *const argument0 = argument; \
  substitute_item(argument,images,images_names,parent_images,parent_images_names,variables_sizes,\
                  command_selection).move_to(_argument); \
  _gmic_substitute_args(argument = _argument,argument0,command,images); \
}

// Macro for computing a readable version of a command argument.
inline char *_gmic_argument_text(const char *const argument, CImg<char>& argument_text, const bool is_verbose) {
  if (is_verbose) return cimg::strellipsize(argument,argument_text,80,false);
  else return &(*argument_text=0);
}

#define gmic_argument_text_printed() _gmic_argument_text(argument,argument_text,is_verbose)
#define gmic_argument_text() _gmic_argument_text(argument,argument_text,true)

// Macro for having 'get' or 'non-get' versions of G'MIC commands.
#define gmic_apply(function) { \
    __ind = (unsigned int)selection[l]; \
    gmic_check(images[__ind]); \
    if (is_double_hyphen) { \
      images[__ind].get_##function.move_to(images); \
      images_names[__ind].get_copymark().move_to(images_names); \
    } else images[__ind].function; \
  }

// Macro for simple commands that has no arguments and act on images.
#define gmic_simple_command(command_name,function,description) \
  if (!std::strcmp(command_name,command)) { \
    print(images,0,description,gmic_selection.data()); \
    cimg_forY(selection,l) gmic_apply(function()); \
    is_released = false; continue; \
}

// Macro for G'MIC arithmetic commands.
#define gmic_arithmetic_command(command_name,\
                                function1,description1,arg1_1,arg1_2,arg1_3,value_type1, \
                                function2,description2,arg2_1,arg2_2, \
                                description3,arg3_1,arg3_2, \
                                description4) \
 if (!std::strcmp(command_name,command)) { \
   gmic_substitute_args(); \
   value = 0; \
   sep = 0; \
   *indices = *formula = 0; \
   if (cimg_sscanf(argument,"%lf%c",&value,&end)==1 || \
       (cimg_sscanf(argument,"%lf%c%c",&value,&sep,&end)==2 && sep=='%')) { \
      const char *const ssep = sep=='%'?"%":""; \
      print(images,0,description1 ".",arg1_1,arg1_2,arg1_3); \
      cimg_forY(selection,l) { \
       CImg<T>& img = gmic_check(images[selection[l]]); \
       nvalue = value; \
       if (sep=='%' && img) { \
         vmax = (double)img.max_min(vmin); \
         nvalue = vmin + (vmax - vmin)*value/100; \
       } \
       if (is_double_hyphen) { \
         int back = 0; \
         images_names.insert(images_names[selection[l]].get_copymark()); \
         images.insert(img); \
         back = (int)images.size() - 1; \
         images[back].function1((value_type1)nvalue); \
       } else img.function1((value_type1)nvalue); \
      } \
      ++position; \
   } else if (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep,&end)==2 && \
              sep==']' \
              && (ind=selection2cimg(indices,images.size(),images_names,command_name,\
                                     true,false,CImg<char>::empty())).  \
              height()==1) { \
     print(images,0,description2 ".",arg2_1,arg2_2); \
     const CImg<T> img0 = gmic_image_arg(*ind); \
     cimg_forY(selection,l) { \
       CImg<T>& img = gmic_check(images[selection[l]]); \
       if (is_double_hyphen) { \
         int back = 0; \
         images_names.insert(images_names[selection[l]].get_copymark()); \
         images.insert(img); \
         back = (int)images.size() - 1; \
         images[back].function2(img0); \
       } else img.function2(img0); \
     } \
     ++position; \
   } else if (cimg_sscanf(argument,"'%4095[^']%c%c",formula,&sep,&end)==2 && sep=='\'') { \
     strreplace_fw(formula); print(images,0,description3 ".",arg3_1,arg3_2); \
     cimg_forY(selection,l) { \
       CImg<T>& img = gmic_check(images[selection[l]]); \
       if (is_double_hyphen) { \
         int back = 0; \
         images_names.insert(images_names[selection[l]].get_copymark()); \
         images.insert(img); \
         back = (int)images.size() - 1; \
         images[back].function2((const char*)formula); \
       } else img.function2((const char*)formula); \
     } \
     ++position; \
   } else { \
     print(images,0,description4 ".",gmic_selection.data()); \
     if (images && selection) { \
       if (is_double_hyphen) { \
         CImg<T> img0 = CImg<T>(gmic_check(images[selection[0]]),false); \
         for (unsigned int l = 1; l<(unsigned int)selection.height(); ++l) \
           img0.function2(gmic_check(images[selection[l]])); \
         images_names.insert(images_names[selection[0]].get_copymark()); \
         img0.move_to(images); \
       } else if (selection.height()>=2) { \
       const unsigned int ind0 = selection[0]; \
       CImg<T>& img0 = gmic_check(images[ind0]); \
       for (unsigned int l = 1; l<(unsigned int)selection.height(); ++l) \
         img0.function2(gmic_check(images[selection[l]])); \
       remove_images(images,images_names,selection,1,selection.height() - 1); \
       }}} is_released = false; continue; \
   }

// Manage mutexes.
struct _gmic_mutex {
#if cimg_OS==2
  HANDLE mutex[256];
  _gmic_mutex() { for (unsigned int i = 0; i<256; ++i) mutex[i] = CreateMutex(0,FALSE,0); }
  void lock(const unsigned int n) { WaitForSingleObject(mutex[n],INFINITE); }
  void unlock(const unsigned int n) { ReleaseMutex(mutex[n]); }
#elif defined(_PTHREAD_H) // #if cimg_OS==2
  pthread_mutex_t mutex[256];
  _gmic_mutex() { for (unsigned int i = 0; i<256; ++i) pthread_mutex_init(&mutex[i],0); }
  void lock(const unsigned int n) { pthread_mutex_lock(&mutex[n]); }
  void unlock(const unsigned int n) { pthread_mutex_unlock(&mutex[n]); }
#else // #if cimg_OS==2
  _gmic_mutex() {}
  void lock(const unsigned int) {}
  void unlock(const unsigned int) {}
#endif // #if cimg_OS==2
};
inline _gmic_mutex& gmic_mutex() { static _gmic_mutex val; return val; }

// Thread structure and routine for command '-parallel'.
template<typename T>
struct st_gmic_parallel {
  CImgList<char> *images_names, *parent_images_names, commands_line;
  CImgList<st_gmic_parallel<T> > *threads_data;
  CImgList<T> *images, *parent_images;
  CImg<unsigned int> variables_sizes;
  const CImg<unsigned int> *command_selection;
  bool is_thread_running;
  gmic_exception exception;
  gmic gmic_instance;
#ifdef gmic_is_parallel
#if cimg_OS!=2
  pthread_t thread_id;
#else // #if cimg_OS!=2
  HANDLE thread_id;
#endif // #if cimg_OS!=2
#endif // #ifdef gmic_is_parallel
  st_gmic_parallel() { variables_sizes.assign(512); }
};

template<typename T>
#if cimg_OS!=2
static void *gmic_parallel(void *arg)
#else // #if cimg_OS!=2
static DWORD WINAPI gmic_parallel(void *arg)
#endif // #if cimg_OS!=2
{
  st_gmic_parallel<T> &st = *(st_gmic_parallel<T>*)arg;
  unsigned int pos = 0;
  try {
    st.gmic_instance.is_debug_info = false;
    st.gmic_instance._run(st.commands_line,pos,*st.images,*st.images_names,
                          *st.parent_images,*st.parent_images_names,
                          st.variables_sizes,0,0,st.command_selection);
  } catch (gmic_exception &e) {

    // Send all remaining running threads the 'abort' signal.
#ifdef gmic_is_parallel
    CImgList<st_gmic_parallel<T> > &threads_data = *st.threads_data;
    cimglist_for(threads_data,i) cimg_forY(threads_data[i],l)
      if (&threads_data(i,l)!=&st && threads_data(i,l).is_thread_running) {
        threads_data(i,l).gmic_instance.is_abort_thread = true;
#if cimg_OS!=2
        pthread_join(threads_data(i,l).thread_id,0);
#else // #if cimg_OS!=2
        WaitForSingleObject(threads_data(i,l).thread_id,INFINITE);
        CloseHandle(threads_data(i,l).thread_id);
#endif // #if cimg_OS!=2
        threads_data(i,l).is_thread_running = false;
      }
#endif // #ifdef gmic_is_parallel

    st.exception._command_help.assign(e._command_help);
    st.exception._message.assign(e._message);
  }
#if defined(gmic_is_parallel) && cimg_OS!=2
  pthread_exit(0);
#endif // #if defined(gmic_is_parallel) && cimg_OS!=2
  return 0;
}

// Return Levenshtein distance between two strings.
// (adapted from http://rosettacode.org/wiki/Levenshtein_distance#C)
int gmic::_levenshtein(const char *const s, const char *const t,
                       CImg<int>& d, const int i, const int j) {
  const int ls = d.width() - 1, lt = d.height() - 1;
  if (d(i,j)>=0) return d(i,j);
  int x;
  if (i==ls) x = lt - j;
  else if (j==lt) x = ls - i;
  else if (s[i]==t[j]) x = _levenshtein(s,t,d,i + 1,j + 1);
  else {
    x = _levenshtein(s,t,d,i + 1,j + 1);
    int y;
    if ((y=_levenshtein(s,t,d,i,j + 1))<x) x = y;
    if ((y=_levenshtein(s,t,d,i + 1,j))<x) x = y;
    ++x;
  }
  return d(i,j) = x;
}

int gmic::levenshtein(const char *const s, const char *const t) {
  const char *const ns = s?s:"", *const nt = t?t:"";
  const int ls = (int)std::strlen(ns), lt = (int)std::strlen(nt);
  if (!ls) return lt; else if (!lt) return ls;
  CImg<int> d(1 + ls,1 + lt,1,1,-1);
  return _levenshtein(ns,nt,d,0,0);
}

// Return true if specified filename corresponds to an existing file or directory.
bool gmic::check_filename(const char *const filename) {
  bool res = false;
#if cimg_OS==2
  const unsigned int attr = (unsigned int)GetFileAttributesA(filename);
  res = (attr!=~0U);
#else // #if cimg_OS==2
  std::FILE *const file = std::fopen(filename,"r");
  if (file) { res = true; std::fclose(file); }
#endif // #if cimg_OS==2
  return res;
}

// Return a 8-bits hashcode from a string.
unsigned int gmic::hashcode(const char *const str, const bool is_variable) {
  if (!str) return 0U;
  unsigned int hash = 0U;
  if (is_variable) {
    if (*str=='_') return str[1]=='_'?511U:510U;
    for (const char *s = str; *s; ++s) (hash*=31)+=*s;
    return hash%510;
  }
  for (const char *s = str; *s; ++s) (hash*=31)+=*s;
  return hash&511;
}

// Tells if the the implementation of a G'MIC command contains arguments.
bool gmic::command_has_arguments(const char *const command) {
  if (!command || !*command) return false;
  for (const char *s = std::strchr(command,'$'); s; s = std::strchr(s,'$')) {
    const char c = *(++s);
    if (c=='#' ||
        c=='*' ||
        c=='=' ||
        (c>'0' && c<='9') ||
        (c=='-' && *(s + 1)>'0' && *(s + 1)<='9') ||
        (c=='\"' && *(s + 1)=='*' && *(s + 2)=='\"') ||
        (c=='{' && (*(s + 1)=='^' ||
                    (*(s + 1)>'0' && *(s + 1)<='9') ||
                    (*(s + 1)=='-' && *(s + 2)>'0' && *(s + 2)<='9')))) return true;
  }
  return false;
}

// Compute the basename of a filename.
const char* gmic::basename(const char *const str)  {
  if (!str) return str;
  const unsigned int l = (unsigned int)std::strlen(str);
  if (*str=='[' && (str[l - 1]==']' || str[l - 1]=='.')) return str;
  const char *p = 0, *np = str;
  while (np>=str && (p=np)) np = std::strchr(np,'/') + 1;
  np = p;
  while (np>=str && (p=np)) np = std::strchr(np,'\\') + 1;
  return p;
}

// Replace special characters in a string.
char *gmic::strreplace_fw(char *const str) {
  if (str) for (char *s = str ; *s; ++s) {
      const char c = *s;
      if (c<' ')
        *s = c==gmic_dollar?'$':c==gmic_lbrace?'{':c==gmic_rbrace?'}':c==gmic_comma?',':
          c==gmic_dquote?'\"':c;
    }
  return str;
}

char *gmic::strreplace_bw(char *const str) {
  if (str) for (char *s = str ; *s; ++s) {
      const char c = *s;
      *s = c=='$'?gmic_dollar:c=='{'?gmic_lbrace:c=='}'?gmic_rbrace:c==','?gmic_comma:
        c=='\"'?gmic_dquote:c;
    }
  return str;
}

// Constructors / destructors.
//----------------------------
#if cimg_display!=0
#define gmic_new_attr commands(new CImgList<char>[512]), commands_names(new CImgList<char>[512]), \
    commands_has_arguments(new CImgList<char>[512]), \
    _variables(new CImgList<char>[512]), _variables_names(new CImgList<char>[512]), \
    variables(new CImgList<char>*[512]), variables_names(new CImgList<char>*[512]), \
    display_window(new CImgDisplay[10]), is_running(false)
#else
#define gmic_new_attr commands(new CImgList<char>[512]), commands_names(new CImgList<char>[512]), \
    commands_has_arguments(new CImgList<char>[512]), \
    _variables(new CImgList<char>[512]), _variables_names(new CImgList<char>[512]), \
    variables(new CImgList<char>*[512]), variables_names(new CImgList<char>*[512]), \
    display_window(0), is_running(false)
#endif // #if cimg_display!=0

CImg<char> gmic::stdlib = CImg<char>::empty();

gmic::gmic():gmic_new_attr {
  CImgList<gmic_pixel_type> images;
  CImgList<char> images_names;
  verbosity = -1;
  _gmic(0,images,images_names,0,true,0,0);
  verbosity = 0;
}

gmic::gmic(const char *const commands_line, const char *const custom_commands,
           const bool include_stdlib, float *const p_progress, bool *const p_is_abort):
  gmic_new_attr {
  CImgList<gmic_pixel_type> images;
  CImgList<char> images_names;
  _gmic(commands_line,
        images,images_names,custom_commands,
        include_stdlib,p_progress,p_is_abort);
}

gmic::~gmic() {
  cimg::exception_mode(cimg_exception_mode);
#if cimg_display!=0
  CImgDisplay *const _display_window = (CImgDisplay*)display_window;
  delete[] _display_window;
#endif
  delete[] commands;
  delete[] commands_names;
  delete[] commands_has_arguments;
  delete[] _variables;
  delete[] _variables_names;
  delete[] variables;
  delete[] variables_names;
}

// Uncompress G'MIC standard library commands.
//---------------------------------------------
const CImg<char>& gmic::uncompress_stdlib() {
  if (!stdlib) try {
      CImgList<char>::get_unserialize(CImg<unsigned char>(data_gmic_stdlib,1,size_data_gmic_stdlib,1,1,true))[0].
        move_to(stdlib);
    } catch (...) {
      cimg::mutex(29);
      std::fprintf(cimg::output(),
                   "[gmic] %s*** Warning *** Could not uncompress G'MIC standard library, ignoring it.%s\n",
                   cimg::t_red,cimg::t_normal);
      std::fflush(cimg::output());
      cimg::mutex(29,0);
      stdlib.assign(1,1,1,1,0);
    }
  return stdlib;
}

// Get path to .gmic user file.
//-----------------------------
const char* gmic::path_user(const char *const custom_path) {
  static CImg<char> path_user;
  if (path_user) return path_user;
  cimg::mutex(28);
  const char *_path_user = 0;
  if (custom_path && cimg::is_directory(custom_path)) _path_user = custom_path;
  if (!_path_user) _path_user = getenv("GMIC_PATH");
  if (!_path_user) _path_user = getenv("GMIC_GIMP_PATH");
  if (!_path_user) {
#if cimg_OS!=2
    _path_user = getenv("HOME");
#else
    _path_user = getenv("APPDATA");
#endif
  }
  if (!_path_user) _path_user = getenv("TMP");
  if (!_path_user) _path_user = getenv("TEMP");
  if (!_path_user) _path_user = getenv("TMPDIR");
  if (!_path_user) _path_user = "";
  path_user.assign(1024);
#if cimg_OS!=2
  cimg_snprintf(path_user,path_user.width(),"%s%c.gmic",
                _path_user,cimg_file_separator);
#else
  cimg_snprintf(path_user,path_user.width(),"%s%cuser.gmic",
                _path_user,cimg_file_separator);
#endif
  CImg<char>::string(path_user).move_to(path_user);  // Optimize length.
  cimg::mutex(28,0);
  return path_user;
}

// Get path to the resource directory.
//------------------------------------
const char* gmic::path_rc(const char *const custom_path) {
  static CImg<char> path_rc;
  CImg<char> path_tmp;
  if (path_rc) return path_rc;
  cimg::mutex(28);
  const char *_path_rc = 0;
  if (custom_path && cimg::is_directory(custom_path)) _path_rc = custom_path;
  if (!_path_rc) _path_rc = getenv("GMIC_PATH");
  if (!_path_rc) _path_rc = getenv("GMIC_GIMP_PATH");
  if (!_path_rc) _path_rc = getenv("XDG_CONFIG_HOME");
  if (!_path_rc) {
#if cimg_OS!=2
    _path_rc = getenv("HOME");
    if (_path_rc) {
      path_tmp.assign(std::strlen(_path_rc) + 10);
      cimg_sprintf(path_tmp,"%s/.config",_path_rc);
      if (cimg::is_directory(path_tmp)) _path_rc = path_tmp;
    }
#else
    _path_rc = getenv("APPDATA");
#endif
  }
  if (!_path_rc) _path_rc = getenv("TMP");
  if (!_path_rc) _path_rc = getenv("TEMP");
  if (!_path_rc) _path_rc = getenv("TMPDIR");
  if (!_path_rc) _path_rc = "";
  path_rc.assign(1024);
  cimg_snprintf(path_rc,path_rc.width(),"%s%cgmic%c",
                _path_rc,cimg_file_separator,cimg_file_separator);
  CImg<char>::string(path_rc).move_to(path_rc);  // Optimize length.
  cimg::mutex(28,0);
  return path_rc;
}

// Create resources directory.
//----------------------------
bool gmic::init_rc(const char *const custom_path) {
  CImg<char> dirname = CImg<char>::string(path_rc(custom_path));
  if (dirname.width()>=2) dirname[dirname.width() - 2] = 0;
  if (!cimg::is_directory(dirname)) {
    std::remove(dirname); // In case 'dirname' is already a file.
#if cimg_OS==2
    return (bool)CreateDirectoryA(dirname,0);
#else
    return !(bool)mkdir(dirname,0777);
#endif
  }
  return true;
}

// Get current call stack as a string.
//-------------------------------------
CImg<char> gmic::callstack2string(const CImg<unsigned int> *const callstack_selection, const bool is_debug) const {
  if (callstack_selection && !*callstack_selection) return CImg<char>("./",3);
  CImgList<char> input_callstack;
  if (!callstack_selection) input_callstack.assign(callstack,true);
  else cimg_forY(*callstack_selection,l) input_callstack.insert(callstack[(*callstack_selection)[l]],~0U,true);
  CImgList<char> res;
  const unsigned int siz = (unsigned int)input_callstack.size();
  if (siz<=9 || is_debug) res.assign(input_callstack,false);
  else {
    res.assign(9);
    res[0].assign(input_callstack[0],false);
    res[1].assign(input_callstack[1],false);
    res[2].assign(input_callstack[2],false);
    res[3].assign(input_callstack[3],false);
    res[4].assign("(...)",6);
    res[5].assign(input_callstack[siz - 4],false);
    res[6].assign(input_callstack[siz - 3],false);
    res[7].assign(input_callstack[siz - 2],false);
    res[8].assign(input_callstack[siz - 1],false);
  }
  cimglist_for(res,l) {
    if (!verbosity && !is_debug) {
      char *const s = res(l,0)!='*'?0:std::strchr(res[l],'#');
      if (s) { *s = 0; CImg<char>(res[l].data(),(unsigned int)(s - res[l].data() + 1)).move_to(res[l]); }
    }
    res[l].back() = '/';
  }
  CImg<char>::vector(0).move_to(res);
  return res>'x';
}

CImg<char> gmic::callstack2string(const bool is_debug) const {
  return callstack2string(0,is_debug);
}

CImg<char> gmic::callstack2string(const CImg<unsigned int>& callstack_selection, const bool is_debug) const {
  return callstack2string(&callstack_selection,is_debug);
}

// Parse items from a G'MIC command line.
//---------------------------------------
CImgList<char> gmic::commands_line_to_CImgList(const char *const commands_line) {
  if (!commands_line || !*commands_line) return CImgList<char>();
  bool is_dquoted = false;
  const char *ptrs0 = commands_line;
  while (*ptrs0==' ') ++ptrs0;  // Remove leading spaces to first item.
  CImg<char> item((unsigned int)std::strlen(ptrs0) + 1);
  CImgList<char> items;
  char *ptrd = item.data(), c = 0;
  for (const char *ptrs = ptrs0; *ptrs; ++ptrs) {
    c = *ptrs;
    if (c=='\\') { // If escaped character.
      c = *(++ptrs);
      if (!c) { c = '\\'; --ptrs; }
      else if (c=='$') c = gmic_dollar;
      else if (c=='{') c = gmic_lbrace;
      else if (c=='}') c = gmic_rbrace;
      else if (c==',') c = gmic_comma;
      else if (c=='\"') c = gmic_dquote;
      else if (c==' ') c = ' ';
      else *(ptrd++) = '\\';
      *(ptrd++) = c;
    } else if (is_dquoted) { // If non-escaped character inside string.
      if (c=='\"') is_dquoted = false;
      else if (c==1) while (c && c!=' ') c = *(++ptrs); // Discard debug info inside string.
      else *(ptrd++) = (c=='$' && ptrs[1]!='?')?gmic_dollar:c=='{'?gmic_lbrace:c=='}'?gmic_rbrace:
             c==','?gmic_comma:c;
    } else { // Non-escaped character outside string.
      if (c=='\"') is_dquoted = true;
      else if (c==' ') {
        *ptrd = 0; CImg<char>(item.data(),(unsigned int)(ptrd - item.data() + 1)).move_to(items);
        ptrd = item.data();
        ++ptrs; while (*ptrs==' ') ++ptrs; ptrs0 = ptrs--;  // Remove trailing spaces to next item.
      } else *(ptrd++) = c;
    }
  }
  if (is_dquoted) {
    CImg<char> str; CImg<char>::string(commands_line).move_to(str); // Discard debug info inside string.
    char *ptrd = str, c = 0;
    bool _is_debug_info = false;
    cimg_for(str,ptrs,char) {
      c = *ptrs;
      if (c && c!=1) *(ptrd++) = c;
      else { // Try to retrieve first debug line when discarding debug info.
        unsigned int _debug_filename = ~0U, _debug_line = ~0U;
        if (!_is_debug_info && cimg_sscanf(ptrs + 1,"%x,%x",&_debug_line,&(_debug_filename=0))) {
          debug_filename = _debug_filename;
          debug_line = _debug_line;
          _is_debug_info = is_debug_info = true;
        }
        while (c && c!=' ') c = *(++ptrs);
      }
    } *ptrd = 0;
    error("Invalid command line: Double quotes are not closed, in expression '%s'.",
          str.data());
  }
  if (ptrd!=item.data() && c!=' ') {
    *ptrd = 0; CImg<char>(item.data(),(unsigned int)(ptrd - item.data() + 1)).move_to(items);
  }
  if (is_debug) {
    debug("Decompose command line into %u items: ",items.size());
    cimglist_for(items,l) {
      if (items(l,0)==1) {
        if (items(l,1)) debug("  item[%u] = (debug info 0x%s)",l,items[l].data() + 1);
        else debug("  item[%u] = (undefined debug info)",l);
      } else debug("  item[%u] = '%s'",l,items[l].data());
    }
  }
  return items;
}

// Print log message.
//-------------------
gmic& gmic::print(const char *format, ...) {
  if (verbosity<0 && !is_debug) return *this;
  va_list ap;
  va_start(ap,format);
  CImg<char> message(65536);
  message[message.width() - 2] = 0;
  cimg_vsnprintf(message,message.width(),format,ap);
  strreplace_fw(message);
  if (message[message.width() - 2]) cimg::strellipsize(message,message.width() - 2);
  va_end(ap);

  // Display message.
  cimg::mutex(29);
  if (*message!='\r')
    for (unsigned int i = 0; i<nb_carriages; ++i) std::fputc('\n',cimg::output());
  nb_carriages = 1;
  std::fprintf(cimg::output(),
               "[gmic]%s %s",
               callstack2string().data(),message.data());
  std::fflush(cimg::output());
  cimg::mutex(29,0);
  return *this;
}

// Print error message, and quit interpreter.
//-------------------------------------------
gmic& gmic::error(const char *const format, ...) {
  va_list ap;
  va_start(ap,format);
  CImg<char> message(1024);
  message[message.width() - 2] = 0;
  cimg_vsnprintf(message,message.width(),format,ap);
  strreplace_fw(message);
  if (message[message.width() - 2]) cimg::strellipsize(message,message.width() - 2);
  va_end(ap);

  // Display message.
  const CImg<char> s_callstack = callstack2string();
  if (verbosity>=0 || is_debug) {
    cimg::mutex(29);
    if (*message!='\r')
      for (unsigned int i = 0; i<nb_carriages; ++i) std::fputc('\n',cimg::output());
    nb_carriages = 1;
    if (is_debug_info && debug_filename<commands_files.size() && debug_line!=~0U)
      std::fprintf(cimg::output(),"[gmic]%s %s%s*** Error (file '%s', %sline #%u) *** %s%s",
                   s_callstack.data(),cimg::t_red,cimg::t_bold,
                   commands_files[debug_filename].data(),
                   is_debug_info?"":"call from ",debug_line,message.data(),
                   cimg::t_normal);
    else
      std::fprintf(cimg::output(),"[gmic]%s %s%s*** Error *** %s%s",
                   s_callstack.data(),cimg::t_red,cimg::t_bold,
                   message.data(),cimg::t_normal);
    std::fflush(cimg::output());
    cimg::mutex(29,0);
  }

  // Store detailled error message for interpreter.
  CImg<char> full_message(512 + message.width());
  if (debug_filename<commands_files.size() && debug_line!=~0U)
    cimg_snprintf(full_message,full_message.width(),
                  "*** Error in %s (file '%s', %sline #%u) *** %s",
                  s_callstack.data(),
                  commands_files[debug_filename].data(),
                  is_debug_info?"":"call from ",debug_line,message.data());
  else cimg_snprintf(full_message,full_message.width(),
                     "*** Error in %s *** %s",
                     s_callstack.data(),message.data());
  CImg<char>::string(full_message).move_to(status);
  message.assign();
  is_running = false;
  throw gmic_exception(0,status);
}

// Print debug message.
//---------------------
gmic& gmic::debug(const char *format, ...) {
  if (!is_debug) return *this;
  va_list ap;
  va_start(ap,format);
  CImg<char> message(1024);
  message[message.width() - 2] = 0;
  cimg_vsnprintf(message,message.width(),format,ap);
  if (message[message.width() - 2]) cimg::strellipsize(message,message.width() - 2);
  va_end(ap);

  // Display message.
  cimg::mutex(29);
  if (*message!='\r')
    for (unsigned int i = 0; i<nb_carriages; ++i) std::fputc('\n',cimg::output());
  nb_carriages = 1;

  if (is_debug_info && debug_filename<commands_files.size() && debug_line!=~0U)
    std::fprintf(cimg::output(),
                 "%s<gmic>%s#%u ",
                 cimg::t_green,callstack2string(true).data(),debug_line);
  else
    std::fprintf(cimg::output(),
                 "%s<gmic>%s ",
                 cimg::t_green,callstack2string(true).data());

  for (char *s = message; *s; ++s) {
    char c = *s;
    if (c<' ') switch (c) {
      case gmic_dollar : std::fprintf(cimg::output(),"\\$"); break;
      case gmic_lbrace : std::fprintf(cimg::output(),"\\{"); break;
      case gmic_rbrace : std::fprintf(cimg::output(),"\\}"); break;
      case gmic_comma : std::fprintf(cimg::output(),"\\,"); break;
      case gmic_dquote : std::fprintf(cimg::output(),"\\\""); break;
      default : std::fputc(c,cimg::output());
      }
    else std::fputc(c,cimg::output());
  }
  std::fprintf(cimg::output(),
               "%s",
               cimg::t_normal);
  std::fflush(cimg::output());
  cimg::mutex(29,0);
  return *this;
}

// Set variable in the interpreter environment.
//---------------------------------------------
// 'operation' can be { 0 (add new variable), '=' (replace or add),'+','-','*','/','%','&','|','^','<','>' }
// Return the variable value.
inline const char * gmic::set_variable(const char *const name, const char *const value,
                                       const char operation,
                                       const unsigned int *const variables_sizes) {
  if (!name || !value) return "";
  char _operation = operation, end;
  bool is_name_found = false;
  double lvalue, rvalue;
  CImg<char> s_value;
  int ind = 0;
  const bool
    is_global = *name=='_',
    is_thread_global = is_global && name[1]=='_';
  if (is_thread_global) cimg::mutex(30);
  const unsigned int hashcode = gmic::hashcode(name,true);
  const int lind = is_global || !variables_sizes?0:(int)variables_sizes[hashcode];
  CImgList<char>
    &__variables = *variables[hashcode],
    &__variables_names = *variables_names[hashcode];
  if (operation) {
    // Retrieve index of current definition.
    for (int l = __variables.width() - 1; l>=lind; --l) if (!std::strcmp(__variables_names[l],name)) {
        is_name_found = true; ind = l; break;
      }
    if (operation=='=') {
      if (!is_name_found) _operation = 0; // New variable.
      else CImg<char>::string(value).move_to(__variables[ind]);
    } else {
      const char *const s_operation = operation=='+'?"+":operation=='-'?"-":operation=='*'?"*":operation=='/'?"/":
        operation=='%'?"%":operation=='&'?"&":operation=='|'?"|":operation=='^'?"^":
        operation=='<'?"<<":">>";
      if (!is_name_found)
        error("Operation '%s=' requested on undefined variable '%s'.",
              s_operation,name);
      if (std::sscanf(__variables[ind],"%lf%c",&lvalue,&end)!=1)
        error("Operation '%s=' requested on non-numerical variable '%s=%s'.",
              s_operation,name,__variables[ind].data());
      if (std::sscanf(value,"%lf%c",&rvalue,&end)!=1)
        error("Operation '%s=' requested on variable '%s', with non-numerical argument '%s'.",
              s_operation,name,value);
      s_value.assign(24); *s_value = 0;
      cimg_snprintf(s_value,s_value.width(),"%.16g",
                    operation=='+'?lvalue + rvalue:
                    operation=='-'?lvalue - rvalue:
                    operation=='*'?lvalue*rvalue:
                    operation=='/'?lvalue/rvalue:
                    operation=='%'?cimg::mod(lvalue,rvalue):
                    operation=='&'?(double)((unsigned long)lvalue & (unsigned long)rvalue):
                    operation=='|'?(double)((unsigned long)lvalue | (unsigned long)rvalue):
                    operation=='^'?std::pow(lvalue,rvalue):
                    operation=='<'?(double)((long)lvalue << (unsigned int)rvalue):
                    (double)((long)lvalue >> (unsigned int)rvalue));
      CImg<char>::string(s_value).move_to(__variables[ind]);
    }
  }
  if (!_operation) { // New variable.
    ind = __variables.width();
    CImg<char>::string(name).move_to(__variables_names);
    CImg<char>::string(value).move_to(__variables);
  }
  if (is_thread_global) cimg::mutex(30,0);
  return __variables[ind].data();
}

// Add custom commands from a char* buffer.
//------------------------------------------
gmic& gmic::add_commands(const char *const data_commands,
                         const char *const commands_file) {
  if (!data_commands || !*data_commands) return *this;
  CImg<char> com(256*1024), line(256*1024), mac(256), debug_info(32);
  CImg<unsigned int> pos(512,1,1,1,0);
  unsigned int line_number = 1;
  bool is_last_slash = false, _is_last_slash = false, is_newline = false;
  int ind = -1, l_debug_info = 0;
  char sep = 0;
  if (commands_file) CImg<char>::string(commands_file).move_to(commands_files);

  for (const char *data = data_commands; *data; is_last_slash = _is_last_slash,
         line_number+=is_newline?1:0) {

    // Read new line.
    char *_line = line, *const line_end = line.end();
    while (*data!='\n' && *data && _line<line_end) *(_line++) = *(data++);
    if (_line<line_end) *_line = 0; else *(line_end - 1) = 0;
    if (*data=='\n') { is_newline = true; ++data; } else is_newline = false; // Skip next '\n'.

    // Replace/remove unusual characters.
    char *__line = line;
    for (_line = line; *_line; ++_line) if (*_line!=13) *(__line++) = (unsigned char)*_line<' '?' ':*_line;
    *__line = 0;
    _line = line; if (*_line=='#') *_line = 0; else do { // Remove comments.
        if ((_line=std::strchr(_line,'#')) && *(_line - 1)==' ') { *--_line = 0; break; }
      } while (_line++);

    // Remove useless trailing spaces.
    char *linee = line.data() + std::strlen(line) - 1;
    while (linee>=line && *linee==' ') --linee; *(linee + 1) = 0;
    char *lines = line; while (*lines==' ') ++lines; // Remove useless leading spaces.
    if (!*lines) continue; // Empty line.

    // Check if last character is a '\'...
    _is_last_slash = false;
    for (_line = linee; *_line=='\\' && _line>=lines; --_line) _is_last_slash = !_is_last_slash;
    if (_is_last_slash) *(linee--) = 0; // .. and remove it if necessary.
    if (!*lines) continue; // Empty line found.
    *mac = *com = 0;

    if (!is_last_slash && std::strchr(lines,':') && // Check for a command definition.
        cimg_sscanf(lines,"%255[a-zA-Z0-9_] %c %262143[^\n]",mac.data(),&sep,com.data())>=2 &&
        (*lines<'0' || *lines>'9') && sep==':') {
      ind = (int)hashcode(mac,false);
      CImg<char>::string(mac).move_to(commands_names[ind],pos[ind]);
      CImg<char> body = CImg<char>::string(com);
      CImg<char>::vector((char)command_has_arguments(body)).
        move_to(commands_has_arguments[ind],pos[ind]);
      if (commands_file) { // Insert code with debug info.
        if (commands_files.width()<2)
          l_debug_info = cimg_snprintf(debug_info.data() + 1,debug_info.width() - 2,"%x",line_number);
        else
          l_debug_info = cimg_snprintf(debug_info.data() + 1,debug_info.width() - 2,"%x,%x",
                                            line_number,commands_files.width() - 1);
        if (l_debug_info>=debug_info.width() - 1) l_debug_info = debug_info.width() - 2;
        debug_info[0] = 1; debug_info[l_debug_info + 1] = ' ';
        ((CImg<char>(debug_info,l_debug_info + 2,1,1,1,true),body)>'x').
          move_to(commands[ind],pos[ind]++);
      } else body.move_to(commands[ind],pos[ind]++); // Insert code without debug info.
    } else { // Continuation of a previous line.
      if (ind<0) error("Command '-command': Syntax error in expression '%s'.",lines);
      const unsigned int p = pos[ind] - 1;
      if (!is_last_slash) commands[ind][p].back() = ' ';
      else --(commands[ind][p]._width);
      const CImg<char> body = CImg<char>(lines,(unsigned int)(linee - lines + 2));
      commands_has_arguments[ind](p,0) |= (char)command_has_arguments(body);
      if (commands_file && !is_last_slash) { // Insert code with debug info.
        if (commands_files.width()<2)
          l_debug_info = cimg_snprintf(debug_info.data() + 1,debug_info.width() - 2,"%x",line_number);
        else
          l_debug_info = cimg_snprintf(debug_info.data() + 1,debug_info.width() - 2,"%x,%x",
                                       line_number,commands_files.width() - 1);
        if (l_debug_info>=debug_info.width() - 1) l_debug_info = debug_info.width() - 2;
        debug_info[0] = 1; debug_info[l_debug_info + 1] = ' ';
        ((commands[ind][p],CImg<char>(debug_info,l_debug_info + 2,1,1,1,true),body)>'x').
          move_to(commands[ind][p]);
      } else commands[ind][p].append(body,'x'); // Insert code without debug info.
    }
  }

  if (is_debug) {
    CImg<unsigned int> hdist(512);
    unsigned int nb_commands = 0;
    cimg_forX(hdist,i) { hdist[i] = commands[i].size(); nb_commands+=commands[i].size(); }
    const CImg<double> st = hdist.get_stats();
    cimg_snprintf(com,com.width(),
                  "Distribution of command hashes: [ %s ], min = %u, max = %u, mean = %g, std = %g.",
                  hdist.value_string().data(),(unsigned int)st[0],(unsigned int)st[1],st[2],
                  std::sqrt(st[3]));
    cimg::strellipsize(com,512,false);
    debug("%s",com.data());
  }
  return *this;
}

// Add commands from a file.
//---------------------------
gmic& gmic::add_commands(std::FILE *const file,
                         const char *const filename) {
  if (!file) return *this;

  // Try reading it first as a .cimg file.
  try {
    CImg<char> buffer;
    buffer.load_cimg(file).unroll('x');
    buffer.resize(buffer.width() + 1,1,1,1,0);
    add_commands(buffer.data(),filename);
  } catch (...) {
    std::rewind(file);
    std::fseek(file,0,SEEK_END);
    const long siz = std::ftell(file);
    std::rewind(file);
    if (siz>0) {
      CImg<char> buffer(siz + 1);
      if (std::fread(buffer.data(),sizeof(char),siz,file)) {
        buffer[siz] = 0;
        add_commands(buffer.data(),filename);
      }
    }
  }
  return *this;
}

// Return subset indices from a selection string.
//-----------------------------------------------
CImg<unsigned int> gmic::selection2cimg(const char *const string, const unsigned int indice_max,
                                        const CImgList<char>& names,
                                        const char *const command, const bool is_selection,
                                        const bool allow_new_name, CImg<char> &new_name) {

  // Try to detect common cases to be faster.
  if (string && !*string) return CImg<unsigned int>(); // Empty selection.
  if (!string || (*string=='^' && !string[1])) { // Whole selection.
    CImg<unsigned int> res(1,indice_max); cimg_forY(res,y) res[y] = (unsigned int)y; return res;
  } else if (*string>='0' && *string<='9' && !string[1]) { // Single digit.
    const unsigned int ind = *string - '0';
    if (ind<indice_max) return CImg<unsigned int>::vector(ind);
  } else if (*string=='-' && string[1]>='0' && string[2]<='9' && !string[2]) { // Single negative digit.
    const unsigned int ind = indice_max - string[1] + '0';
    if (ind<indice_max) return CImg<unsigned int>::vector(ind);
  }

  // Manage remaining cases.
  const char *const stype = is_selection?"selection":"subset";
  const int
    ctypel = is_selection?'[':'\'',
    ctyper = is_selection?']':'\'';

  CImg<bool> is_selected(1,indice_max,1,1,false);
  bool is_inverse = *string=='^';
  const char *it = string + (is_inverse?1:0);
  for (bool stopflag = false; !stopflag; ) {
    CImg<char> name(256), item;
    float ind0 = 0, ind1 = 0, step = 1;
    int iind0 = 0, iind1 = 0;
    bool is_label = false;
    char sep = 0;

    const char *const it_comma = std::strchr(it,',');
    if (it_comma) { item.assign(it,(unsigned int)(it_comma - it + 1)); item.back() = 0; it = it_comma + 1; }
    else { CImg<char>::string(it).move_to(item); stopflag = true; }

    char end, *const it_colon = std::strchr(item,':');
    if (it_colon) {
      *it_colon = 0;
      if (cimg_sscanf(it_colon + 1,"%f%c",&step,&end)!=1 || step<=0)
        error("Command '%s': Invalid %s %c%s%c (syntax error after colon ':').",
              command,stype,ctypel,string,ctyper);
    }
    if (!*item) { // Particular cases [:N] or [^:N].
      if (is_inverse) { iind0 = 0; iind1 = -1; is_inverse = false; }
      else continue;
    } else if (cimg_sscanf(item,"%f%c",&ind0,&end)==1) { // Single indice.
      iind1 = iind0 = (int)cimg::round(ind0);
    } else if (cimg_sscanf(item,"%f-%f%c",&ind0,&ind1,&end)==2) { // Sequence between 2 indices.
      iind0 = (int)cimg::round(ind0);
      iind1 = (int)cimg::round(ind1);
    } else if (cimg_sscanf(item,"%255[a-zA-Z0-9_]%c",name.data(),&end)==1 && // Label.
               (*name<'0' || *name>'9')) {
      cimglist_for(names,l) if (names[l] && !std::strcmp(names[l],name)) {
        is_selected(l) = true; is_label = true;
      }
      if (!is_label) {
        if (allow_new_name && !new_name) {
          iind0 = iind1 = -1;
          CImg<char>::string(name).move_to(new_name);
        } else error("Command '%s': Invalid %s %c%s%c (undefined label '%s').",
                     command,stype,ctypel,string,ctyper,name.data());
      }
    } else if (cimg_sscanf(item,"%f%c%c",&ind0,&sep,&end)==2 && sep=='%') { // Single percent.
      iind1 = iind0 = (int)cimg::round(ind0*((int)indice_max - 1)/100) - (ind0<0?1:0);
    } else if (cimg_sscanf(item,"%f%%-%f%c%c",&ind0,&ind1,&sep,&end)==3 && sep=='%') {
      // Sequence between 2 percents.
      iind0 = (int)cimg::round(ind0*((int)indice_max - 1)/100) - (ind0<0?1:0);
      iind1 = (int)cimg::round(ind1*((int)indice_max - 1)/100) - (ind1<0?1:0);
    } else if (cimg_sscanf(item,"%f%%-%f%c",&ind0,&ind1,&end)==2) {
      // Sequence between a percent and an indice.
      iind0 = (int)cimg::round(ind0*((int)indice_max - 1)/100) - (ind0<0?1:0);;
      iind1 = (int)cimg::round(ind1);
    } else if (cimg_sscanf(item,"%f-%f%c%c",&ind0,&ind1,&sep,&end)==3 && sep=='%') {
      // Sequence between an indice and a percent.
      iind0 = (int)cimg::round(ind0);
      iind1 = (int)cimg::round(ind1*((int)indice_max - 1)/100) - (ind1<0?1:0);;
    } else error("Command '%s': Invalid %s %c%s%c.",
                 command,stype,ctypel,string,ctyper);

    if (!indice_max) error("Command '%s': Invalid %s %c%s%c (no data available).",
                           command,stype,ctypel,string,ctyper);
    if (!is_label) {
      int
        uind0 = (int)(iind0<0?iind0 + indice_max:iind0),
        uind1 = (int)(iind1<0?iind1 + indice_max:iind1);
      if (uind0>uind1) { cimg::swap(uind0,uind1); cimg::swap(iind0,iind1); }
      if (uind0<0 || uind0>=(int)indice_max)
        error("Command '%s': Invalid %s %c%s%c (contains indice '%d', "
              "not in range -%u...%u).",
              command,stype,ctypel,string,ctyper,iind0,indice_max,indice_max - 1);
      if (uind1<0 || uind1>=(int)indice_max)
        error("Command '%s': Invalid %s %c%s%c (contains indice '%d', "
              "not in range -%u...%u).",
              command,stype,ctypel,string,ctyper,iind1,indice_max,indice_max - 1);
      const int istep = (int)cimg::round(step);
      for (int l = uind0; l<=uind1; l+=istep) is_selected[l] = true;
    }
  }
  unsigned int indice = 0;
  cimg_for(is_selected,p,bool) if (*p) ++indice;
  CImg<unsigned int> selection(1,is_inverse?indice_max - indice:indice);
  indice = 0;
  if (is_inverse) { cimg_forY(is_selected,l) if (!is_selected[l]) selection[indice++] = (unsigned int)l; }
  else cimg_forY(is_selected,l) if (is_selected[l]) selection[indice++] = (unsigned int)l;
  return selection;
}

// Return selection or filename strings from a set of indices.
//------------------------------------------------------------
// output_type can be { 0=display indices without brackets | 1=display indices with brackets | 2=display image names }
CImg<char>& gmic::selection2string(const CImg<unsigned int>& selection,
                                   const CImgList<char>& images_names,
                                   const unsigned int output_type,
                                   CImg<char>& res) const {
  res.assign(256);
  if (output_type<2) {
    const char *const bl = output_type?"[":"", *const br = output_type?"]":"";
    switch (selection.height()) {
    case 0:
      cimg_snprintf(res.data(),res.width()," %s%s",bl,br);
      break;
    case 1:
      cimg_snprintf(res.data(),res.width()," %s%u%s",
                    bl,selection[0],br);
      break;
    case 2:
      cimg_snprintf(res.data(),res.width(),"s %s%u,%u%s",
                    bl,selection[0],selection[1],br);
      break;
    case 3:
      cimg_snprintf(res.data(),res.width(),"s %s%u,%u,%u%s",
                    bl,selection[0],selection[1],selection[2],br);
      break;
    case 4:
      cimg_snprintf(res.data(),res.width(),"s %s%u,%u,%u,%u%s",
                    bl,selection[0],selection[1],selection[2],selection[3],br);
      break;
    case 5:
      cimg_snprintf(res.data(),res.width(),"s %s%u,%u,%u,%u,%u%s",
                    bl,selection[0],selection[1],selection[2],selection[3],selection[4],br);
      break;
    case 6:
      cimg_snprintf(res.data(),res.width(),"s %s%u,%u,%u,%u,%u,%u%s",
                    bl,selection[0],selection[1],selection[2],
                    selection[3],selection[4],selection[5],br);
      break;
    case 7:
      cimg_snprintf(res.data(),res.width(),"s %s%u,%u,%u,%u,%u,%u,%u%s",
                    bl,selection[0],selection[1],selection[2],selection[3],
                    selection[4],selection[5],selection[6],br);
      break;
    default:
      cimg_snprintf(res.data(),res.width(),"s %s%u,%u,%u,(...),%u,%u,%u%s",
                    bl,selection[0],selection[1],selection[2],
                    selection[selection.height() - 3],
                    selection[selection.height() - 2],
                    selection[selection.height() - 1],br);
    }
    return res;
  }

  switch (selection.height()) {
  case 0:
    *res = 0;
    break;
  case 1:
    cimg_snprintf(res.data(),res.width(),"%s",
                  basename(images_names[selection[0]].data()));
    break;
  case 2:
    cimg_snprintf(res.data(),res.width(),"%s, %s",
                  basename(images_names[selection[0]].data()),
                  basename(images_names[selection[1]].data()));
    break;
  case 3:
    cimg_snprintf(res.data(),res.width(),"%s, %s, %s",
                  basename(images_names[selection[0]].data()),
                  basename(images_names[selection[1]].data()),
                  basename(images_names[selection[2]].data()));
    break;
  case 4:
    cimg_snprintf(res.data(),res.width(),"%s, %s, %s, %s",
                  basename(images_names[selection[0]].data()),
                  basename(images_names[selection[1]].data()),
                  basename(images_names[selection[2]].data()),
                  basename(images_names[selection[3]].data()));
    break;
  default:
    cimg_snprintf(res.data(),res.width(),"%s, (...), %s",
                  basename(images_names[selection[0]].data()),
                  basename(images_names[selection.back()].data()));
  }
  return res;
}

// Print log message.
//-------------------
template<typename T>
gmic& gmic::print(const CImgList<T>& list, const CImg<unsigned int> *const callstack_selection,
                  const char *format, ...) {
  if (verbosity<0 && !is_debug) return *this;
  va_list ap;
  va_start(ap,format);
  CImg<char> message(65536);
  message[message.width() - 2] = 0;
  cimg_vsnprintf(message,message.width(),format,ap);
  strreplace_fw(message);
  if (message[message.width() - 2]) cimg::strellipsize(message,message.width() - 2);
  va_end(ap);

  // Display message.
  cimg::mutex(29);
  if (*message!='\r')
    for (unsigned int i = 0; i<nb_carriages; ++i) std::fputc('\n',cimg::output());
  nb_carriages = 1;
  if (!callstack_selection || *callstack_selection)
    std::fprintf(cimg::output(),
                 "[gmic]-%u%s %s",
                 list.size(),callstack2string(callstack_selection).data(),message.data());
  else std::fprintf(cimg::output(),"%s",message.data());
  std::fflush(cimg::output());
  cimg::mutex(29,0);
  return *this;
}

// Print warning message.
//-----------------------
template<typename T>
gmic& gmic::warn(const CImgList<T>& list, const CImg<unsigned int> *const callstack_selection,
                 const bool force_visible, const char *const format, ...) {
  if (!force_visible && verbosity<0 && !is_debug) return *this;
  va_list ap;
  va_start(ap,format);
  CImg<char> message(1024);
  message[message.width() - 2] = 0;
  cimg_vsnprintf(message,message.width(),format,ap);
  strreplace_fw(message);
  if (message[message.width() - 2]) cimg::strellipsize(message,message.width() - 2);
  va_end(ap);

  // Display message.
  const CImg<char> s_callstack = callstack2string(callstack_selection);
  cimg::mutex(29);
  if (*message!='\r')
    for (unsigned int i = 0; i<nb_carriages; ++i) std::fputc('\n',cimg::output());
  nb_carriages = 1;
  if (!callstack_selection || *callstack_selection) {
    if (debug_filename<commands_files.size() && debug_line!=~0U)
      std::fprintf(cimg::output(),
                   "[gmic]-%u%s %s%s*** Warning (file '%s', %sline #%u) *** %s%s",
                   list.size(),s_callstack.data(),cimg::t_magenta,cimg::t_bold,
                   commands_files[debug_filename].data(),
                   is_debug_info?"":"call from ",debug_line,message.data(),
                   cimg::t_normal);
    else
      std::fprintf(cimg::output(),
                   "[gmic]-%u%s %s%s*** Warning *** %s%s",
                   list.size(),s_callstack.data(),cimg::t_magenta,cimg::t_bold,
                   message.data(),cimg::t_normal);
  } else std::fprintf(cimg::output(),"%s%s%s%s",
                      cimg::t_magenta,cimg::t_bold,message.data(),cimg::t_normal);
  std::fflush(cimg::output());
  cimg::mutex(29,0);
  return *this;
}

// Print error message, and quit interpreter.
//-------------------------------------------
template<typename T>
gmic& gmic::error(const CImgList<T>& list, const CImg<unsigned int> *const callstack_selection,
                  const char *const command, const char *const format, ...) {
  va_list ap;
  va_start(ap,format);
  CImg<char> message(1024);
  message[message.width() - 2] = 0;
  cimg_vsnprintf(message,message.width(),format,ap);

  strreplace_fw(message);
  if (message[message.width() - 2]) cimg::strellipsize(message,message.width() - 2);
  va_end(ap);

  // Display message.
  const CImg<char> s_callstack = callstack2string(callstack_selection);
  if (verbosity>=0 || is_debug) {
    cimg::mutex(29);
    if (*message!='\r')
      for (unsigned int i = 0; i<nb_carriages; ++i) std::fputc('\n',cimg::output());
    nb_carriages = 1;
    if (!callstack_selection || *callstack_selection) {
      if (debug_filename<commands_files.size() && debug_line!=~0U)
        std::fprintf(cimg::output(),
                     "[gmic]-%u%s %s%s*** Error (file '%s', %sline #%u) *** %s%s",
                     list.size(),s_callstack.data(),cimg::t_red,cimg::t_bold,
                     commands_files[debug_filename].data(),
                     is_debug_info?"":"call from ",debug_line,message.data(),
                     cimg::t_normal);
      else
        std::fprintf(cimg::output(),
                     "[gmic]-%u%s %s%s*** Error *** %s%s",
                     list.size(),s_callstack.data(),cimg::t_red,cimg::t_bold,
                     message.data(),cimg::t_normal);
    } else std::fprintf(cimg::output(),"%s",message.data());
    std::fflush(cimg::output());
    cimg::mutex(29,0);
  }

  // Store detailled error message for interpreter.
  CImg<char> full_message(512 + message.width());
  if (debug_filename<commands_files.size() && debug_line!=~0U)
    cimg_snprintf(full_message,full_message.width(),
                  "*** Error in %s (file '%s', %sline #%u) *** %s",
                  s_callstack.data(),
                  commands_files[debug_filename].data(),
                  is_debug_info?"":"call from ",debug_line,message.data());
  else cimg_snprintf(full_message,full_message.width(),
                     "*** Error in %s *** %s",
                     s_callstack.data(),message.data());
  CImg<char>::string(full_message).move_to(status);
  message.assign();
  is_running = false;
  throw gmic_exception(command,status);
}

#define arg_error(command) gmic::error(images,0,command,"Command '-%s': Invalid argument '%s'.",\
                                       command,gmic_argument_text())

// Print debug message.
//---------------------
template<typename T>
gmic& gmic::debug(const CImgList<T>& list, const char *format, ...) {
  if (!is_debug) return *this;
  va_list ap;
  va_start(ap,format);
  CImg<char> message(1024);
  message[message.width() - 2] = 0;
  cimg_vsnprintf(message,message.width(),format,ap);
  if (message[message.width() - 2]) cimg::strellipsize(message,message.width() - 2);
  va_end(ap);

  // Display message.
  cimg::mutex(29);
  if (*message!='\r')
    for (unsigned int i = 0; i<nb_carriages; ++i) std::fputc('\n',cimg::output());
  nb_carriages = 1;
  if (is_debug_info && debug_filename!=~0U && debug_line!=~0U)
    std::fprintf(cimg::output(),
                 "%s<gmic>-%u%s#%u ",
                 cimg::t_green,list.size(),callstack2string(true).data(),debug_line);
  else
    std::fprintf(cimg::output(),
                 "%s<gmic>-%u%s ",
                 cimg::t_green,list.size(),callstack2string(true).data());
  for (char *s = message; *s; ++s) {
    char c = *s;
    if (c<' ') {
      switch (c) {
      case gmic_dollar : std::fprintf(cimg::output(),"\\$"); break;
      case gmic_lbrace : std::fprintf(cimg::output(),"\\{"); break;
      case gmic_rbrace : std::fprintf(cimg::output(),"\\}"); break;
      case gmic_comma : std::fprintf(cimg::output(),"\\,"); break;
      case gmic_dquote : std::fprintf(cimg::output(),"\\\""); break;
      default : std::fputc(c,cimg::output());
      }
    } else std::fputc(c,cimg::output());
  }
  std::fprintf(cimg::output(),
               "%s",
               cimg::t_normal);
  std::fflush(cimg::output());
  cimg::mutex(29,0);
  return *this;
}

// Check if a shared image of the image list is safe or not.
//----------------------------------------------------------
template<typename T>
inline bool gmic_is_valid_pointer(const T *const ptr) {
#if cimg_OS==1
  const int result = access((const char*)ptr,F_OK);
  if (result==-1 && errno==EFAULT) return false;
#elif cimg_OS==2 // #if cimg_OS==1
  return !IsBadReadPtr((void*)ptr,1);
#endif // #if cimg_OS==1
  return true;
}

template<typename T>
CImg<T>& gmic::check_image(const CImgList<T>& list, CImg<T>& img) {
  check_image(list,(const CImg<T>&)img);
  return img;
}

template<typename T>
const CImg<T>& gmic::check_image(const CImgList<T>& list, const CImg<T>& img) {
#ifdef gmic_check_image
  if (!img.is_shared() || gmic_is_valid_pointer(img.data())) return img;
  if (is_debug) error(list,0,0,"Image list contains an invalid shared image (%p,%d,%d,%d,%d) "
                      "(references a deallocated buffer).",
                      img.data(),img.width(),img.height(),img.depth(),img.spectrum());
  else error(list,0,0,"Image list contains an invalid shared image (%d,%d,%d,%d) "
             "(references a deallocated buffer).",
             img.width(),img.height(),img.depth(),img.spectrum());
#else // #ifdef gmic_check_image
  cimg::unused(list);
#endif // #ifdef gmic_check_image
  return img;
}

#define gmic_check(img) check_image(images,img)

// Remove list of images in a selection.
//---------------------------------------
template<typename T>
gmic& gmic::remove_images(CImgList<T> &images, CImgList<char> &images_names,
                          const CImg<unsigned int>& selection,
                          const unsigned int start, const unsigned int end) {
  if (start==0 && end==(unsigned int)selection.height() - 1 && selection.height()==images.width()) {
    images.assign();
    images_names.assign();
  } else for (int l = (int)end; l>=(int)start; ) {
      unsigned int eind = selection[l--], ind = eind;
      while (l>=(int)start && selection[l]==ind - 1) ind = selection[l--];
      images.remove(ind,eind); images_names.remove(ind,eind);
    }
  return *this;
}

// Template constructor.
//----------------------
template<typename T>
gmic::gmic(const char *const commands_line, CImgList<T>& images, CImgList<char>& images_names,
           const char *const custom_commands, const bool include_stdlib,
           float *const p_progress, bool *const p_is_abort):gmic_new_attr {
  _gmic(commands_line,
        images,images_names,
        custom_commands,include_stdlib,
        p_progress,p_is_abort);
}

// This method is shared by all constructors. It initializes all the interpreter environment.
template<typename T>
void gmic::_gmic(const char *const commands_line,
                 CImgList<T>& images, CImgList<char>& images_names,
                 const char *const custom_commands, const bool include_stdlib,
                 float *const p_progress, bool *const p_is_abort) {

  // Initialize class variables and default G'MIC environment.
  setlocale(LC_NUMERIC,"C");
  cimg_exception_mode = cimg::exception_mode();
  cimg::exception_mode(0);
  cimg::srand();
  is_debug = false;
  is_double3d = true;
  nb_carriages = 0;
  verbosity = 0;
  render3d = 4;
  renderd3d = -1;
  focale3d = 700;
  light3d.assign();
  light3d_x = light3d_y = 0;
  light3d_z = -5e8f;
  specular_lightness3d = 0.15f;
  specular_shininess3d = 0.8f;
  starting_commands_line = commands_line;
  reference_time = cimg::time();
  for (unsigned int l = 0; l<512; ++l) {
    commands_names[l].assign();
    commands[l].assign();
    commands_has_arguments[l].assign();
    _variables[l].assign();
    variables[l] = &_variables[l];
    _variables_names[l].assign();
    variables_names[l] = &_variables_names[l];
  }
  if (include_stdlib) add_commands(gmic::uncompress_stdlib().data());
  add_commands(custom_commands);

  // Set pre-defined global variables.
  CImg<char> str(8);
  cimg_snprintf(str,str.width(),"%u",cimg::nb_cpus());
  set_variable("_cpus",str,0);

#if cimg_OS==1
  cimg_snprintf(str,str.width(),"%u",(unsigned int)getpid());
#elif cimg_OS==2 // #if cimg_OS==1
  cimg_snprintf(str,str.width(),"%u",(unsigned int)_getpid());
#else // #if cimg_OS==1
  cimg_snprintf(str,str.width(),"0");
#endif // #if cimg_OS==1
  set_variable("_pid",str,0);

#ifdef gmic_prerelease
  set_variable("_prerelease",gmic_prerelease,0);
#endif // #ifdef gmic_prerelease

  cimg_snprintf(str,str.width(),"%u",gmic_version);
  set_variable("_version",str,0);

  set_variable("_path_rc",gmic::path_rc(),0);
  set_variable("_path_user",gmic::path_user(),0);

#ifdef cimg_use_vt100
  set_variable("_vt100","1",0);
#endif // # if cimg_use_vt100

  // Launch the G'MIC interpreter.
  const CImgList<char> items = commands_line?commands_line_to_CImgList(commands_line):CImgList<char>::empty();
  try {
    _run(items,images,images_names,p_progress,p_is_abort);
  } catch (gmic_exception&) {
    print(images,0,"Abort G'MIC interpreter.\n");
    throw;
  }
}

// Print info on selected images.
//--------------------------------
template<typename T>
gmic& gmic::print_images(const CImgList<T>& images, const CImgList<char>& images_names,
                         const CImg<unsigned int>& selection, const bool is_header) {
  if (!images || !images_names || !selection) {
    if (is_header) print(images,0,"Print image [].");
    return *this;
  }
  const bool is_verbose = verbosity>=0 || is_debug;

  CImg<char> title(256);
  if (is_header) {
    CImg<char> gmic_selection, gmic_names;
    if (is_verbose) {
      selection2string(selection,images_names,1,gmic_selection);
      selection2string(selection,images_names,2,gmic_names);
    }
    cimg::strellipsize(gmic_names,80,false);
    print(images,0,"Print image%s = '%s'.\n",
          gmic_selection.data(),gmic_names.data());
  }
  if (is_verbose) {
    cimg_forY(selection,l) {
      const unsigned int uind = selection[l];
      const CImg<T>& img = images[uind];
      bool is_valid = true;
      int _verbosity = verbosity;
      bool _is_debug = is_debug;
      verbosity = -1; is_debug = false;
      try { gmic_check(img); } catch (gmic_exception&) { is_valid = false; }
      verbosity = _verbosity; is_debug = _is_debug;
      cimg_snprintf(title,title.width(),"[%u] = '%s'",
                    uind,images_names[uind].data());
      cimg::strellipsize(title,80,false);
      img.gmic_print(title,is_debug,is_valid);
    }
    nb_carriages = 0;
  }
  return *this;
}

// Display selected images.
//-------------------------
template<typename T>
gmic& gmic::display_images(const CImgList<T>& images, const CImgList<char>& images_names,
                           const CImg<unsigned int>& selection, unsigned int *const XYZ,
                           const bool exit_on_anykey) {
#if cimg_display!=0
  CImgDisplay *const _display_window = (CImgDisplay*)display_window;
#endif
  if (!images || !images_names || !selection) { print(images,0,"Display image []."); return *this; }
  const bool is_verbose = verbosity>=0 || is_debug;
  CImg<char> gmic_selection;
  if (is_verbose) selection2string(selection,images_names,1,gmic_selection);

  // Check for available display.
#if cimg_display==0
  cimg::unused(exit_on_anykey);
  print(images,0,"Display image%s",gmic_selection.data());
  if (is_verbose) {
    cimg::mutex(29);
    if (XYZ) std::fprintf(cimg::output(),", from point (%u,%u,%u)",XYZ[0],XYZ[1],XYZ[2]);
    std::fprintf(cimg::output()," (console output only, no display support).\n");
    std::fflush(cimg::output());
    cimg::mutex(29,0);
    print_images(images,images_names,selection,false);
  }
#else // #if cimg_display==0
  bool is_available_display = false;
  try {
    is_available_display = (bool)CImgDisplay::screen_width();
  } catch (CImgDisplayException&) {
    print(images,0,"Display image%s",gmic_selection.data());
    if (is_verbose) {
      cimg::mutex(29);
      if (XYZ) std::fprintf(cimg::output(),", from point (%u,%u,%u)",XYZ[0],XYZ[1],XYZ[2]);
      std::fprintf(cimg::output()," (console output only, no display available).\n");
      std::fflush(cimg::output());
      cimg::mutex(29,0);
      print_images(images,images_names,selection,false);
    }
  }
  if (!is_available_display) return *this;

  CImgList<T> visu;
  CImg<bool> is_valid(1,selection.height(),1,1,true);
  cimg_forY(selection,l) {
    const CImg<T>& img = images[selection[l]];
    int _verbosity = verbosity;
    bool _is_debug = is_debug;
    verbosity = -1; is_debug = false;
    try { gmic_check(img); } catch (gmic_exception&) { is_valid[l] = false; }
    verbosity = _verbosity; is_debug = _is_debug;
  }

  cimg_forY(selection,l) {
    const unsigned int uind = selection[l];
    const CImg<T>& img = images[uind];
    if (img && is_valid[l]) visu.insert(img,~0U,true);
    else visu.insert(1);
  }

  CImg<char> gmic_names;
  if (visu) selection2string(selection,images_names,2,gmic_names);
  cimg::strellipsize(gmic_names,80,false);

  print(images,0,"Display image%s = '%s'",gmic_selection.data(),gmic_names.data());
  if (is_verbose) {
    cimg::mutex(29);
    if (XYZ) std::fprintf(cimg::output(),", from point (%u,%u,%u).\n",XYZ[0],XYZ[1],XYZ[2]);
    else std::fprintf(cimg::output(),".\n");
    std::fflush(cimg::output());
    nb_carriages = 0;
    cimg::mutex(29,0);
  }
  if (visu) {
    CImgDisplay _disp, &disp = _display_window[0]?_display_window[0]:_disp;
    CImg<char> title(256);
    if (visu.size()==1)
      cimg_snprintf(title,title.width(),"%s (%dx%dx%dx%d)",
                    gmic_names.data(),
                    visu[0].width(),visu[0].height(),visu[0].depth(),visu[0].spectrum());
    else
      cimg_snprintf(title,title.width(),"%s (%u)",
                    gmic_names.data(),visu.size());
    cimg::strellipsize(title,80,false);
    CImg<bool> is_shared(visu.size());
    cimglist_for(visu,l) {
      is_shared[l] = visu[l].is_shared();
      visu[l]._is_shared = images[selection[l]].is_shared();
    }
    print_images(images,images_names,selection,false);
    if (disp) visu.display(disp.set_title("%s",title.data()),false,'x',0.5f,XYZ,exit_on_anykey);
    else visu.display(title.data(),false,'x',0.5f,XYZ,exit_on_anykey);
    cimglist_for(visu,l) visu[l]._is_shared = is_shared(l);
  }
#endif // #if cimg_display==0
  return *this;
}

// Display plots of selected images.
//----------------------------------
template<typename T>
gmic& gmic::display_plots(const CImgList<T>& images, const CImgList<char>& images_names,
                          const CImg<unsigned int>& selection,
                          const unsigned int plot_type, const unsigned int vertex_type,
                          const double xmin, const double xmax,
                          const double ymin, const double ymax,
                          const bool exit_on_anykey) {
#if cimg_display!=0
  CImgDisplay *const _display_window = (CImgDisplay*)display_window;
#endif
  if (!images || !images_names || !selection) { print(images,0,"Plot image []."); return *this; }
  const bool is_verbose = verbosity>=0 || is_debug;
  CImg<char> gmic_selection;
  if (is_verbose) selection2string(selection,images_names,1,gmic_selection);

#if cimg_display==0
  print(images,0,"Plot image%s (console output only, no display support).\n",gmic_selection.data());
  print_images(images,images_names,selection,false);
  cimg::unused(plot_type,vertex_type,xmin,xmax,ymin,ymax,exit_on_anykey);
#else // #if cimg_display==0
  bool is_available_display = false;
  try {
    is_available_display = (bool)CImgDisplay::screen_width();
  } catch (CImgDisplayException&) {
    print(images,0,"Plot image%s (console output only, no display available).",gmic_selection.data());
    print_images(images,images_names,selection,false);
  }
  if (!is_available_display) return *this;

  CImgList<unsigned int> empty_indices;
  cimg_forY(selection,l) if (!gmic_check(images[selection(l)]))
    CImg<unsigned int>::vector(selection(l)).move_to(empty_indices);
  if (empty_indices && is_verbose) {
    CImg<char> eselec;
    selection2string(empty_indices>'y',images_names,1,eselec);
    warn(images,0,false,
         "Command '-plot': Image%s %s empty.",
         eselec.data(),empty_indices.size()>1?"are":"is");
  }

  CImg<char> gmic_names;
  if (is_verbose) selection2string(selection,images_names,2,gmic_names);
  print(images,0,"Plot image%s = '%s'.",
        gmic_selection.data(),gmic_names.data());

  CImgDisplay _disp, &disp = _display_window[0]?_display_window[0]:_disp;
  bool is_first_line = false;
  cimg_forY(selection,l) {
    const unsigned int uind = selection[l];
    const CImg<T>& img = images[uind];
    if (img) {
      if (is_verbose && !is_first_line) {
        cimg::mutex(29);
        std::fputc('\n',cimg::output());
        std::fflush(cimg::output());
        cimg::mutex(29,0);
        is_first_line = true;
      }
      img.print(images_names[uind].data());
      if (!disp) disp.assign(cimg_fitscreen(CImgDisplay::screen_width()/2,CImgDisplay::screen_height()/2,1),0,0);
      img.display_graph(disp.set_title("%s (%dx%dx%dx%d)",
                                       basename(images_names[uind].data()),
                                       img.width(),img.height(),img.depth(),img.spectrum()),
                        plot_type,vertex_type,0,xmin,xmax,0,ymin,ymax,exit_on_anykey);
      if (is_verbose) nb_carriages = 0;
    }
  }
#endif // #if cimg_display==0
  return *this;
}

// Display selected 3d objects.
//-----------------------------
template<typename T>
gmic& gmic::display_objects3d(const CImgList<T>& images, const CImgList<char>& images_names,
                              const CImg<unsigned int>& selection,
                              const CImg<unsigned char>& background3d,
                              const bool exit_on_anykey) {
#if cimg_display!=0
  CImgDisplay *const _display_window = (CImgDisplay*)display_window;
#endif
  if (!images || !images_names || !selection) {
    print(images,0,"Display 3d object [].");
    return *this;
  }

  const bool is_verbose = verbosity>=0 || is_debug;
  CImg<char> gmic_selection;
  if (is_verbose) selection2string(selection,images_names,1,gmic_selection);

  CImg<char> error_message(1024);
  cimg_forY(selection,l) if (!gmic_check(images[selection[l]]).is_CImg3d(true,error_message))
    error(images,0,0,
          "Command '-display3d': Invalid 3d object [%d] in selected image%s (%s).",
          selection[l],gmic_selection_err.data(),error_message.data());
#if cimg_display==0
  print(images,0,"Display 3d object%s (skipped, no display support).",gmic_selection.data());
  cimg::unused(background3d,exit_on_anykey);
#else // #if cimg_display==0
  bool is_available_display = false;
  try {
    is_available_display = (bool)CImgDisplay::screen_width();
  } catch (CImgDisplayException&) {
    print(images,0,"Display 3d object%s (skipped, no display available).",gmic_selection.data());
  }
  if (!is_available_display) return *this;

  CImgDisplay _disp, &disp = _display_window[0]?_display_window[0]:_disp;
  cimg_forY(selection,l) {
    const unsigned int uind = selection[l];
    const CImg<T>& img = images[uind];
    if (!disp) {
      if (background3d) disp.assign(cimg_fitscreen(background3d.width(),background3d.height(),1),0,0);
      else disp.assign(cimg_fitscreen(CImgDisplay::screen_width()/2,CImgDisplay::screen_height()/2,1),0,0);
    }

    CImg<unsigned char> background;
    if (background3d) background = background3d.get_resize(disp.width(),disp.height(),1,3);
    else background.assign(1,2,1,3).fill(32,64,32,116,64,96).resize(1,256,1,3,3).
           resize(disp.width(),disp.height(),1,3);
    background.display(disp);

    CImgList<unsigned int> primitives;
    CImgList<unsigned char> colors;
    CImgList<float> opacities;
    CImg<float> vertices(img,false), pose3d(4,4,1,1,0);
    pose3d(0,0) = pose3d(1,1) = pose3d(2,2) = pose3d(3,3) = 1;
    vertices.CImg3dtoobject3d(primitives,colors,opacities,false);
    print(images,0,"Display 3d object [%u] = '%s' (%d vertices, %u primitives).",
          uind,images_names[uind].data(),
          vertices.width(),primitives.size());
    disp.set_title("%s (%d vertices, %u primitives)",
                   basename(images_names[uind].data()),
                   vertices.width(),primitives.size());
    if (light3d) colors.insert(light3d,~0U,true);
    background.display_object3d(disp,vertices,primitives,colors,opacities,
                                true,render3d,renderd3d,is_double3d,focale3d,
                                light3d_x,light3d_y,light3d_z,
                                specular_lightness3d,specular_shininess3d,
                                true,pose3d,exit_on_anykey);
    print(images,0,"Selected 3d pose = [ %g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g ].",
          pose3d[0],pose3d[1],pose3d[2],pose3d[3],
          pose3d[4],pose3d[5],pose3d[6],pose3d[7],
          pose3d[8],pose3d[9],pose3d[10],pose3d[11],
          pose3d[12],pose3d[13],pose3d[14],pose3d[15]);
    if (disp.is_closed()) break;
  }
#endif // #if cimg_display==0
  return *this;
}

// Substitute '{}' and '$' expressions in a string.
//--------------------------------------------------
template<typename T>
CImg<char> gmic::substitute_item(const char *const source,
                                 CImgList<T>& images, CImgList<char>& images_names,
                                 CImgList<T>& parent_images, CImgList<char>& parent_images_names,
                                 const unsigned int *const variables_sizes,
                                 const CImg<unsigned int> *const command_selection) {
#if cimg_display!=0
  CImgDisplay *const _display_window = (CImgDisplay*)display_window;
#endif
  if (!source) return CImg<char>();
  CImg<char> substituted_items, inbraces, substr(40);
  CImg<unsigned int> _ind;

  for (const char *nsource = source; *nsource; )
    if (*nsource!='{' && *nsource!='$') {
      // If not starting with '{', or '$'.
      const char *const nsource0 = nsource;
      do { ++nsource; } while (*nsource && *nsource!='{' && *nsource!='$');
      CImg<char>(nsource0,(unsigned int)(nsource - nsource0),1,1,1,true).append_string_to(substituted_items);
    } else { // '{...}' or '${...}' expression found.
      bool is_braces = false, is_substituted = false;
      int ind = 0, l_inbraces = 0;
      char sep = 0;
      _ind.assign();
      *substr = 0;
      if (inbraces) *inbraces = 0; else inbraces.assign(1,1,1,1,0);

      // '{...}' expression.
      if (*nsource=='{') {
        const char *const ptr_beg = nsource + 1, *ptr_end = ptr_beg;
        unsigned int p = 0;
        for (p = 1; p>0 && *ptr_end; ++ptr_end) { if (*ptr_end=='{') ++p; if (*ptr_end=='}') --p; }
        if (p) { CImg<char>(nsource++,1).append_string_to(substituted_items); continue; }
        l_inbraces = (int)(ptr_end - ptr_beg - 1);

        if (l_inbraces>0) {
          inbraces.assign(ptr_beg,l_inbraces + 1).back() = 0;
          substitute_item(inbraces,images,images_names,parent_images,parent_images_names,variables_sizes,
                          command_selection).move_to(inbraces);
          strreplace_fw(inbraces);
        }
        nsource+=l_inbraces + 2;
        if (!*inbraces)
          error(images,0,0,
                "Item substitution '{}': Empty braces.");

        // Display window features.
        if (!is_substituted && *inbraces=='*' &&
            (!inbraces[1] ||
             (inbraces[1]>='0' && inbraces[1]<='9' && !inbraces[2]) ||
             (inbraces[1]==',' && inbraces[2]) ||
             (inbraces[1]>='0' && inbraces[1]<='9' && inbraces[2]==',' && inbraces[3]))) {
#if cimg_display==0
          *substr = '0'; substr[1] = 0;
          is_substituted = true;
#else // #if cimg_display==0
          unsigned int wind = 0;
          const char *feature = inbraces.data() + 1;
          if (*feature>='0' && *feature<='9') wind = (unsigned int)(*(feature++) - '0');
          CImgDisplay &disp = _display_window[wind];

          if (!*feature) {
            cimg_snprintf(substr,substr.width(),"%d",disp?(disp.is_closed()?0:1):0);
            is_substituted = true;
          } else if (*feature==',') {
            bool flush_request = false;
            if (*(++feature)=='-' &&
                feature[1]!='w' && feature[1]!='h' && feature[1]!='d' && feature[1]!='e' &&
                feature[1]!='u' && feature[1]!='v' && feature[1]!='n' && feature[1]!='t') {
              flush_request = true; ++feature;
            }
            if (!feature[1]) switch (*feature) { // Single-char features.
              case 'w' : // Display width.
                cimg_snprintf(substr,substr.width(),"%d",disp.width());
                is_substituted = true;
                break;
              case 'h' : // Display height.
                cimg_snprintf(substr,substr.width(),"%d",disp.height());
                is_substituted = true;
                break;
              case 'd' : // Window width.
                cimg_snprintf(substr,substr.width(),"%d",disp.window_width());
                is_substituted = true;
                break;
              case 'e' : // Window height.
                cimg_snprintf(substr,substr.width(),"%d",disp.window_height());
                is_substituted = true;
                break;
              case 'u' : // Screen width.
                try {
                  cimg_snprintf(substr,substr.width(),"%d",CImgDisplay::screen_width());
                } catch (CImgDisplayException&) {
                  *substr = '0'; substr[1] = 0;
                }
                is_substituted = true;
                break;
              case 'v' : // Screen height.
                try {
                  cimg_snprintf(substr,substr.width(),"%d",CImgDisplay::screen_height());
                } catch (CImgDisplayException&) {
                  *substr = '0'; substr[1] = 0;
                }
                is_substituted = true;
                break;
              case 'n' : // Normalization type.
                cimg_snprintf(substr,substr.width(),"%d",disp.normalization());
                is_substituted = true;
                break;
              case 't' : // Window title.
                cimg_snprintf(substr,substr.width(),"%s",disp.title());
                is_substituted = true;
                break;
              case 'x' : // X-coordinate of mouse pointer.
                cimg_snprintf(substr,substr.width(),"%d",disp.mouse_x());
                is_substituted = true;
                if (flush_request) { disp._mouse_x = -1; disp._mouse_y = -1; }
                break;
              case 'y' : // Y-coordinate of mouse pointer.
                cimg_snprintf(substr,substr.width(),"%d",disp.mouse_y());
                is_substituted = true;
                if (flush_request) { disp._mouse_x = -1; disp._mouse_y = -1; }
                break;
              case 'b' : // State of mouse buttons.
                cimg_snprintf(substr,substr.width(),"%d",disp.button());
                is_substituted = true;
                if (flush_request) disp._button = 0;
                break;
              case 'o' : // State of mouse wheel.
                cimg_snprintf(substr,substr.width(),"%d",disp.wheel());
                is_substituted = true;
                if (flush_request) disp._wheel = 0;
                break;
              case 'c' : // Closed state of display window.
                cimg_snprintf(substr,substr.width(),"%d",(int)disp.is_closed());
                is_substituted = true;
                if (flush_request) disp._is_closed = false;
                break;
              case 'r' : // Resize event.
                cimg_snprintf(substr,substr.width(),"%d",(int)disp.is_resized());
                is_substituted = true;
                if (flush_request) disp._is_resized = false;
                break;
              case 'm' : // Move event.
                cimg_snprintf(substr,substr.width(),"%d",(int)disp.is_moved());
                is_substituted = true;
                if (flush_request) disp._is_moved = false;
                break;
              case 'k' : // Key event.
                cimg_snprintf(substr,substr.width(),"%u",disp.key());
                is_substituted = true;
                if (flush_request) disp._keys[0] = 0;
                break;
              } else if (*feature=='w' && feature[1]=='h' && !feature[2]) { // Display width*height.
              cimg_snprintf(substr,substr.width(),"%ld",
                            (long)disp.width()*disp.height());
              is_substituted = true;
            } else if (*feature=='d' && feature[1]=='e' && !feature[2]) { // Window width*height.
              cimg_snprintf(substr,substr.width(),"%ld",
                            (long)disp.window_width()*disp.window_height());
              is_substituted = true;
            } else if (*feature=='u' && feature[1]=='v' && !feature[2]) { // Screen width*height.
              try {
                cimg_snprintf(substr,substr.width(),"%ld",
                              (long)CImgDisplay::screen_width()*CImgDisplay::screen_height());
              } catch (CImgDisplayException&) {
                *substr = '0'; substr[1] = 0;
              }
              is_substituted = true;
            }
            if (!is_substituted) { // Pressed state of specified key.
              bool &ik = disp.is_key(feature);
              cimg_snprintf(substr,substr.width(),"%d",(int)ik);
              is_substituted = true;
              if (flush_request) ik = false;
            }
          }
#endif // #if cimg_display==0
        }

        // Sequence of ascii characters.
        if (!is_substituted && inbraces.width()>=3 && *inbraces=='\'' &&
            inbraces[inbraces.width() - 2]=='\'') {
          const char *s = inbraces.data() + 1;
          if (inbraces.width()>3) {
            inbraces[inbraces.width() - 2] = 0;
            for (*substr = 0, cimg::strunescape(inbraces); *s; ++s) {
              cimg_snprintf(substr,substr.width(),"%d,",(int)(unsigned char)*s);
              CImg<char>(substr.data(),(unsigned int)std::strlen(substr)).append_string_to(substituted_items);
            }
            if (*substr) --(substituted_items._width);
          }
          *substr = 0; is_substituted = true;
        }

        // Sequence of ascii codes.
        if (!is_substituted && inbraces.width()>=3 && *inbraces=='`' &&
            inbraces[inbraces.width() - 2]=='`') {
          if (inbraces.width()>3) {
            unsigned int nb_values = 1;
            cimg_for(inbraces,p,char) if (*p==',') ++nb_values;
            inbraces[inbraces.width() - 2] = 0;
            try {
              CImg<char>(nb_values,1,1,1).fill(inbraces.data() + 1,false,false).append_string_to(substituted_items);
              is_substituted = true;
            } catch (CImgException &e) {
              const char *const e_ptr = std::strstr(e.what(),": ");
              error(images,0,0,
                    "Item substitution '{`%s`}': %s",
                    cimg::strellipsize(inbraces.data() + 1,64,false),e_ptr?e_ptr + 2:e.what());
            }
          }
          *substr = 0; is_substituted = true;
        }

        // String comparison.
        if (!is_substituted && inbraces.width()>=5) {
          char *const peq = std::strstr(inbraces,"'=='");
          if (peq) {
            *peq = 0;
            cimg::strunescape(inbraces.data());
            cimg::strunescape(peq + 4);
            cimg_snprintf(substr,substr.width(),"%d",(int)!std::strcmp(inbraces,peq + 4));
            is_substituted = true;
          } else {
            char *const pne = std::strstr(inbraces,"'!='");
            if (pne) {
              *pne = 0;
              cimg::strunescape(inbraces.data());
              cimg::strunescape(pne + 4);
              cimg_snprintf(substr,substr.width(),"%d",(int)std::strcmp(inbraces,pne + 4));
              is_substituted = true;
            }
          }
        }

        // Image feature.
        if (!is_substituted) {
          const char *feature = inbraces;
          if (l_inbraces<=2) ind = images.width() - 1; // Single-char case.
          else if (cimg_sscanf(inbraces,"%d%c",&ind,&(sep=0))==2 && sep==',') {
            if (ind<0) ind+=images.width();
            if (ind<0 || ind>=images.width()) {
              if (images.width())
                error(images,0,0,
                      "Item substitution '{%s}': Invalid selection [%d] (not in range -%u...%u).",
                      cimg::strellipsize(inbraces,64,false),ind,images.size(),images.size() - 1);
              else
                error(images,0,0,
                      "Item substitution '{%s}': Invalid selection [%d] (no image data available).",
                      cimg::strellipsize(inbraces,64,false),ind);
            }
            while (*feature!=',') ++feature; ++feature;
          } else if (cimg_sscanf(inbraces,"%255[a-zA-Z0-9_]%c",substr.assign(256).data(),&(sep=0))==2 && sep==',') {
            selection2cimg(substr,images.size(),images_names,
                           "Item substitution '{name,feature}'",true,
                           false,CImg<char>::empty()).move_to(_ind);
            if (_ind.height()!=1)
              error(images,0,0,
                    "Item substitution '{%s}': Invalid selection [%s], specifies multiple images.",
                    cimg::strellipsize(inbraces,64,false),substr.data());
            ind = (int)*_ind;
            while (*feature!=',') ++feature; ++feature;
          } else ind = images.width() - 1;

          CImg<T> &img = ind>=0?gmic_check(images[ind]):CImg<T>::empty();
          *substr = 0;
          if (!*feature)
            error(images,0,0,
                  "Item substitution '{%s}': Request for empty feature.",
                  cimg::strellipsize(inbraces,64,false));

          if (!feature[1]) switch (*feature) { // Single-char feature.
            case 'b' : { // Image basename.
              if (ind>=0) {
                substr.assign(cimg::max(substr.width(),images_names[ind].width()));
                cimg::split_filename(images_names[ind].data(),substr);
                const char *const basename = gmic::basename(substr);
                std::memmove(substr,basename,std::strlen(basename) + 1);
                strreplace_bw(substr);
              }
              is_substituted = true;
            } break;
            case 'c' : // Coordinates of minimal value.
              if (img) _ind = img.get_stats(); else _ind.assign(8,1,1,1,0);
              cimg_snprintf(substr,substr.width(),"%u,%u,%u,%u",
                            _ind[4],_ind[5],_ind[6],_ind[7]);
              is_substituted = true;
              break;
            case 'C' : // Coordinates of maximal value.
              if (img) _ind = img.get_stats(); else _ind.assign(12,1,1,1,0);
              cimg_snprintf(substr,substr.width(),"%u,%u,%u,%u",
                            _ind[8],_ind[9],_ind[10],_ind[11]);
              is_substituted = true;
              break;
            case 'd' : // Image depth.
              cimg_snprintf(substr,substr.width(),"%d",img.depth());
              is_substituted = true;
              break;
            case 'f' : { // Image folder name.
              if (ind>=0) {
                substr.assign(cimg::max(substr.width(),images_names[ind].width()));
                std::strcpy(substr,images_names[ind]);
                const char *const basename = gmic::basename(substr);
                substr[basename - substr.data()] = 0;
                strreplace_bw(substr);
              }
              is_substituted = true;
            } break;
            case 'h' : // Image height.
              cimg_snprintf(substr,substr.width(),"%d",img.height());
              is_substituted = true;
              break;
            case 'n' : // Image name.
              if (ind>=0) {
                substr.assign(cimg::max(substr.width(),images_names[ind].width()));
                cimg_snprintf(substr,substr.width(),"%s",images_names[ind].data());
                strreplace_bw(substr);
              }
              is_substituted = true;
              break;
            case 's' : // Number of image channels.
              cimg_snprintf(substr,substr.width(),"%d",img.spectrum());
              is_substituted = true;
              break;
            case 't' : { // Ascii string from image values.
              const unsigned int siz = (unsigned int)img.size();
              if (siz) {
                unsigned int strsiz = 0;
                cimg_for(img,ptr,T) if ((unsigned char)*ptr) ++strsiz; else break;
                if (strsiz) {
                  CImg<char> text(strsiz + 1), _text = text.get_shared_points(0,strsiz - 1,0,0,0);
                  _text = CImg<T>(img.data(),strsiz,1,1,1,true);
                  text.back() = 0;
                  strreplace_bw(text);
                  _text.append_string_to(substituted_items);
                }
              }
              *substr = 0; is_substituted = true;
            } break;
            case 'x' : // Image extension.
              if (ind>=0) {
                substr.assign(cimg::max(substr.width(),images_names[ind].width()));
                cimg_snprintf(substr,substr.width(),"%s",
                              cimg::split_filename(images_names[ind].data()));
                strreplace_bw(substr);
              }
              is_substituted = true;
              break;
            case 'w' : // Image width.
              cimg_snprintf(substr,substr.width(),"%d",img.width());
              is_substituted = true;
              break;
            case '^' : { // Sequence of all pixel values.
              CImg<char> vs = img.value_string(',');
              if (vs && *vs) { --vs._width; vs.append_string_to(substituted_items); }
              *substr = 0; is_substituted = true;
            } break;
            }

          const unsigned int l_feature = (unsigned int)std::strlen(feature);
          if (!is_substituted && *feature=='[' && feature[l_feature - 1]==']') { // Subset of values.
            if (l_feature>=2) {
              CImg<char> subset(feature + 1,l_feature - 1);
              subset.back() = 0;
              CImg<T> values;
              ++feature;
              int _verbosity = verbosity;
              bool _is_debug = is_debug;
              verbosity = -1; is_debug = false;
              CImg<char> _status;
              status.move_to(_status); // Save status because 'selection2cimg' may change it.
              try {
                const CImg<unsigned int>
                  inds = selection2cimg(subset,img.size(),
                                        CImgList<char>::empty(),"",false,false,CImg<char>::empty());
                values.assign(1,inds.height());
                cimg_foroff(inds,p) values[p] = img[inds(p)];
              } catch (gmic_exception &e) {
                const char *const e_ptr = std::strstr(e.what(),": ");
                error(images,0,0,
                      "Item substitution '{%s}': %s",
                      cimg::strellipsize(inbraces,64,false),e_ptr?e_ptr + 2:e.what());
              }
              _status.move_to(status);
              verbosity = _verbosity; is_debug = _is_debug;
              cimg_foroff(values,p) {
                cimg_snprintf(substr,substr.width(),"%.16g",(double)values[p]);
                CImg<char>::string(substr).append_string_to(substituted_items).back() = ',';
              }
              if (values) --(substituted_items._width);
            }
            *substr = 0; is_substituted = true;
          }

          if (!is_substituted) { // Other mathematical expression.
            const bool is_rounded = *feature=='_';
            if (is_rounded) ++feature;
            try {
              CImg<double> output;
              img.eval(output,feature,0,0,0,0,&images,&images);
              if (output._height>1) { // Vector-valued result
                CImg<char> vs = output.value_string(',',0,is_rounded?"%g":"%.16g");
                if (vs && *vs) { --vs._width; vs.append_string_to(substituted_items); }
              } else { // Scalar result
                if (is_rounded) cimg_snprintf(substr,substr.width(),"%g",*output);
                else cimg_snprintf(substr,substr.width(),"%.16g",*output);
                is_substituted = true;
              }
            } catch (CImgException& e) {
              const char *const e_ptr = std::strstr(e.what(),": ");
              error(images,0,0,
                    "Item substitution '{%s}': %s",
                    cimg::strellipsize(inbraces,64,false),e_ptr?e_ptr + 2:e.what());
            }
          }
        }
        if (is_substituted && *substr)
          CImg<char>(substr.data(),(unsigned int)std::strlen(substr)).append_string_to(substituted_items);
        continue;

        //  '${..}' expressions.
      } else if (nsource[1]=='{') {
        const char *const ptr_beg = nsource + 2, *ptr_end = ptr_beg; unsigned int p = 0;
        for (p = 1; p>0 && *ptr_end; ++ptr_end) { if (*ptr_end=='{') ++p; if (*ptr_end=='}') --p; }
        if (p) { CImg<char>(nsource++,1).append_string_to(substituted_items); continue; }
        l_inbraces = (int)(ptr_end - ptr_beg - 1);
        if (l_inbraces>0) {
          inbraces.assign(ptr_beg,l_inbraces + 1).back() = 0;
          substitute_item(inbraces,images,images_names,parent_images,parent_images_names,variables_sizes,
                          command_selection).move_to(inbraces);
        }
        is_braces = true;
      }

      // Substitute '$?' -> String that describes the current command selection.
      if (nsource[1]=='?') {
        if (command_selection) {
          const unsigned int substr_width = (unsigned int)substr.width();
          selection2string(*command_selection,images_names,1,substr);
          CImg<char>(substr.data(),(unsigned int)std::strlen(substr)).append_string_to(substituted_items);
          substr.assign(substr_width);
          nsource+=2;
        } else  CImg<char>(nsource++,1).append_string_to(substituted_items);

        // Substitute '$!' -> Number of images in the list.
      } else if (nsource[1]=='!') {
        cimg_snprintf(substr,substr.width(),"%u",images.size());
        CImg<char>(substr.data(),(unsigned int)std::strlen(substr)).append_string_to(substituted_items);
        nsource+=2;

        // Substitute '$^' -> Verbosity level.
      } else if (nsource[1]=='^') {
        cimg_snprintf(substr,substr.width(),"%d",verbosity);
        CImg<char>(substr.data(),(unsigned int)std::strlen(substr)).append_string_to(substituted_items);
        nsource+=2;

        // Substitute '$|' -> Timer value.
      } else if (nsource[1]=='|') {
        cimg_snprintf(substr,substr.width(),"%g",(cimg::time() - reference_time)/1000.);
        CImg<char>(substr.data(),(unsigned int)std::strlen(substr)).append_string_to(substituted_items);
        nsource+=2;

        // Substitute '$/' -> Current call stack.
      } else if (nsource[1]=='/') {
        cimglist_for(callstack,i) callstack[i].append_string_to(substituted_items).back() = '/';
        nsource+=2;

        // Substitute '$>' and '$<' -> Forward/backward indice of current loop.
      } else if (nsource[1]=='>' || nsource[1]=='<') {
        if (!repeatdones)
          error(images,0,0,
                "Item substitution '$%c': There is no loop currently running.",
                nsource[1]);
        const CImg<unsigned int> &rd = repeatdones.back();
        cimg_snprintf(substr,substr.width(),"%u",nsource[1]=='>'?rd[2]:rd[1] - 1);
        CImg<char>(substr.data(),(unsigned int)std::strlen(substr)).append_string_to(substituted_items);
        nsource+=2;

        // Substitute '$name' and '${name}' -> Variable, image indice or environment variable.
      } else if ((((is_braces && cimg_sscanf(inbraces,"%255[a-zA-Z0-9_]",
                                             substr.assign(256).data())==1) &&
                   !inbraces[std::strlen(substr)]) ||
                  (cimg_sscanf(nsource + 1,"%255[a-zA-Z0-9_]",substr.assign(256).data())==1)) &&
                 (*substr<'0' || *substr>'9')) {
        const CImg<char>& name = is_braces?inbraces:substr;
        const unsigned int
          hashcode = gmic::hashcode(name,true),
          l_name = is_braces?l_inbraces + 3:std::strlen(name) + 1;
        const bool
          is_global = *name=='_',
          is_thread_global = is_global && name[1]=='_';
        const int lind = is_global?0:(int)variables_sizes[hashcode];
        if (is_thread_global) cimg::mutex(30);
        const CImgList<char>
          &__variables = *variables[hashcode],
          &__variables_names = *variables_names[hashcode];
        bool is_name_found = false;
        for (int l = __variables.width() - 1; l>=lind; --l)
          if (!std::strcmp(__variables_names[l],name)) {
            is_name_found = true; ind = l; break;
          }
        if (is_name_found) {
          if (__variables[ind].size()>1)
            CImg<char>(__variables[ind].data(),__variables[ind].size() - 1).
              append_string_to(substituted_items);
        } else {
          for (int l = images.width() - 1; l>=0; --l)
            if (images_names[l] && !std::strcmp(images_names[l],name)) {
              is_name_found = true; ind = l; break;
            }
          if (is_name_found) {
            cimg_snprintf(substr,substr.width(),"%d",ind);
            CImg<char>(substr.data(),(unsigned int)std::strlen(substr)).append_string_to(substituted_items);
          } else {
            const char *const s_env = std::getenv(name);
            if (s_env) CImg<char>(s_env,(unsigned int)std::strlen(s_env)).append_string_to(substituted_items);
          }
        }
        if (is_thread_global) cimg::mutex(30,0);
        nsource+=l_name;

        // Substitute '${"-command"}' -> Status value after command execution.
      } else if (is_braces) {
        nsource+=l_inbraces + 3;
        if (l_inbraces>0) {
          const CImgList<char>
            ncommands_line = commands_line_to_CImgList(strreplace_fw(inbraces));
          unsigned int nposition = 0;
          CImg<char>::string("*substitute").move_to(callstack);
          CImg<unsigned int> nvariables_sizes(512);
          cimg_forX(nvariables_sizes,l) nvariables_sizes[l] = variables[l]->size();
          _run(ncommands_line,nposition,images,images_names,parent_images,parent_images_names,
               nvariables_sizes,0,inbraces,command_selection);
          for (unsigned int l = 0; l<nvariables_sizes._width - 2; ++l) if (variables[l]->size()>nvariables_sizes[l]) {
              variables_names[l]->remove(nvariables_sizes[l],variables[l]->size() - 1);
              variables[l]->remove(nvariables_sizes[l],variables[l]->size() - 1);
            }
          callstack.remove();
          is_return = false;
        }
        if (status.width()>1)
          CImg<char>(status.data(),(unsigned int)std::strlen(status)).append_string_to(substituted_items);

        // Replace '$' by itself.
      } else CImg<char>(nsource++,1).append_string_to(substituted_items);
    }
  substituted_items.resize(substituted_items.width() + 1,1,1,1,0);
  return substituted_items;
}

// Main parsing procedures.
//-------------------------
gmic& gmic::run(const char *const commands_line,
                float *const p_progress, bool *const p_is_abort) {
  gmic_list<gmic_pixel_type> images;
  gmic_list<char> images_names;
  return run(commands_line,images,images_names,
             p_progress,p_is_abort);
}

template<typename T>
gmic& gmic::run(const char *const commands_line,
                gmic_list<T> &images, gmic_list<char> &images_names,
                float *const p_progress, bool *const p_is_abort) {
  cimg::mutex(26);
  if (is_running)
    error(images,0,0,
          "An instance of G'MIC interpreter %p is already running.",
          (void*)this);
  is_running = true;
  cimg::mutex(26,0);
  starting_commands_line = commands_line;
  is_debug = false;
  _run(commands_line_to_CImgList(commands_line),
       images,images_names,p_progress,p_is_abort);
  is_running = false;
  return *this;
}

template<typename T>
gmic& gmic::_run(const gmic_list<char>& commands_line,
                 gmic_list<T> &images, gmic_list<char> &images_names,
                 float *const p_progress, bool *const p_is_abort) {
  CImg<unsigned int> variables_sizes(512,1,1,1,0);
  unsigned int position = 0;
  setlocale(LC_NUMERIC,"C");
  callstack.assign(1U);
  callstack._data[0].assign(2,1,1,1);
  callstack._data[0]._data[0] = '.';
  callstack._data[0]._data[1] = 0;
  dowhiles.assign(0U);
  repeatdones.assign(0U);
  status.assign(0U);
  nb_carriages = 0;
  debug_filename = ~0U;
  debug_line = ~0U;
  is_released = true;
  is_debug_info = false;
  is_debug = false;
  is_start = true;
  is_quit = false;
  is_return = false;
  check_elif = false;
  if (p_progress) progress = p_progress; else { _progress = -1; progress = &_progress; }
  if (p_is_abort) is_abort = p_is_abort; else { _is_abort = false; is_abort = &_is_abort; }
  is_abort_thread = false;
  *progress = -1;
  cimglist_for(commands_line,l) if (!std::strcmp("-debug",commands_line[l].data())) { is_debug = true; break; }
  return _run(commands_line,position,images,images_names,images,images_names,variables_sizes,0,0,0);
}

#ifdef _MSC_VER
#pragma optimize("y", off)
#endif // #ifdef _MSC_VER

template<typename T>
gmic& gmic::_run(const CImgList<char>& commands_line, unsigned int& position,
                 CImgList<T>& images, CImgList<char>& images_names,
                 CImgList<T>& parent_images, CImgList<char>& parent_images_names,
                 const unsigned int *const variables_sizes,
                 bool *const is_noarg, const char *const parent_arguments,
                 const CImg<unsigned int> *const command_selection) {

#if cimg_display!=0
  CImgDisplay *const _display_window = (CImgDisplay*)display_window;
#endif
  if (!commands_line || position>=commands_line._width) {
    if (is_debug) debug(images,"Return from empty function '%s/'.",
                        callstack.back().data());
    return *this;
  }
  typedef typename cimg::superset<T,float>::type Tfloat;
  typedef typename cimg::superset<T,long>::type Tlong;
  const unsigned int initial_callstack_size = callstack.size(), initial_debug_line = debug_line;

  CImgList<st_gmic_parallel<T> > threads_data;
  CImgList<unsigned int> primitives;
  CImgList<unsigned char> g_list_uc;
  CImgList<float> g_list_f;
  CImgList<char> g_list_c;
  CImgList<T> g_list;

  CImg<unsigned int> ind, ind0, ind1;
  CImg<unsigned char> g_img_uc;
  CImg<float> vertices;
  CImg<char> name;
  CImg<T> g_img;

  unsigned int next_debug_line = ~0U, next_debug_filename = ~0U, _debug_line, _debug_filename,
    __ind = 0, boundary = 0, pattern = 0, exit_on_anykey = 0;
  char end, sep = 0, sep0 = 0, sep1 = 0, sepx = 0, sepy = 0, sepz = 0, sepc = 0, axis = 0;
  double vmin = 0, vmax = 0, value, value0, value1, nvalue, nvalue0, nvalue1;
  bool is_endlocal = false;
  float opacity = 0;

  // Allocate string variables, widely used afterwards
  // (prevents stack overflow on recursive calls while remaining thread-safe).
  CImg<char> _formula(4096), _color(4096), message(1024), _title(256), _indices(256),
    _argx(256), _argy(256), _argz(256), _argc(256), _current_command(256), argument_text(81),
    _command(256), _restriction(256);

  char
    *const formula = _formula.data(),
    *const color = _color.data(),
    *const title = _title.data(),
    *const indices = _indices.data(),
    *const argx = _argx.data(),
    *const argy = _argy.data(),
    *const argz = _argz.data(),
    *const argc = _argc.data(),
    *const current_command = _current_command.data(),
    *const command = _command.data(),
    *const restriction = _restriction.data();
  *formula = *color = *title = *indices = *argx = *argy = *argz = *argc =
    *current_command = *command = *restriction = 0;

  try {

    // Init interpreter environment.
    if (images.size()<images_names.size())
      images_names.remove(images.size(),images_names.size() - 1);
    else if (images.size()>images_names.size())
      images_names.insert(images.size() - images_names.size(),CImg<char>::string("[unnamed]"));

    if (is_debug) {
      if (is_start) {
        print(images,0,"Start G'MIC interpreter (in debug mode).");
        debug(images,"Initial command line: '%s'.",starting_commands_line);
        commands_line_to_CImgList(starting_commands_line); // Do it twice, when debug enabled.
      }
      nb_carriages = 2;
      debug(images,"%sEnter scope '%s/'.%s",
            cimg::t_bold,callstack.back().data(),cimg::t_normal);
      is_start = false;
    }

    // Begin command line parsing.
    if (!commands_line && is_start) { print(images,0,"Start G'MIC interpreter."); is_start = false; }
    while (position<commands_line.size() && !is_quit && !is_return) {

#ifdef cimg_use_abort
      _cimg_is_abort.ptr = is_abort;
#endif // #ifdef cimg_use_abort
      const bool is_first_item = !position;

      // Process debug info.
      if (next_debug_line!=~0U) { debug_line = next_debug_line; next_debug_line = ~0U; }
      if (next_debug_filename!=~0U) { debug_filename = next_debug_filename; next_debug_filename = ~0U; }
      while (position<commands_line.size() && *commands_line[position]==1) {
        if (cimg_sscanf(commands_line[position].data() + 1,"%x,%x",&_debug_line,&(_debug_filename=0))>0) {
          is_debug_info = true; debug_line = _debug_line; debug_filename = _debug_filename;
        }
        ++position;
      }
      if (position>=commands_line.size()) break;

      // Check consistency of the interpreter environment.
      if (images_names.size()!=images.size())
        error("G'MIC encountered a fatal error (images (%u) and images names (%u) have different size). "
               "Please submit a bug report, at: https://github.com/dtschump/gmic/issues",
              images_names.size(),images.size());
      if (!callstack)
        error("G'MIC encountered a fatal error (empty call stack). "
              "Please submit a bug report, at: https://github.com/dtschump/gmic/issues");
      if (callstack.size()>=64)
        error("Call stack overflow (infinite recursion?).");

      // Substitute expressions in current item.
      const char
        *const initial_item = commands_line[position].data(),
        *const empty_argument = "",
        *initial_argument = empty_argument;

      unsigned int position_argument = position + 1;
      while (position_argument<commands_line.size() && *(commands_line[position_argument])==1) {
        if (cimg_sscanf(commands_line[position_argument].data() + 1,"%x,%x",&_debug_line,&(_debug_filename=0))>0) {
          is_debug_info = true; next_debug_line = _debug_line; next_debug_filename = _debug_filename;
        }
        ++position_argument;
      }
      if (position_argument<commands_line.size()) initial_argument = commands_line[position_argument];

      CImg<char> _item, _argument;
      substitute_item(initial_item,images,images_names,parent_images,parent_images_names,
                      variables_sizes,command_selection).move_to(_item);
      char *item = _item;
      const char *argument = initial_argument;

      // Split command/restriction, if necessary.
      *command = *restriction = 0;

      bool is_double_hyphen = false, is_restriction = false;
      const unsigned int siz = images._width;
      CImg<unsigned int> selection;
      CImg<char> new_name;
      if (*item=='-' && item[1] && item[1]!='.') {
        sep0 = sep1 = 0;
        if (item[1]=='-' && item[2] && item[2]!='[' && item[2]!='.' && (item[2]!='3' || item[3]!='d')) {
          ++item; is_double_hyphen = true;
        }
        strreplace_fw(item);
        int err = cimg_sscanf(item,"%255[^[]%c%255[a-zA-Z_0-9.eE%^,:+-]%c%c",
                                    command,&sep0,restriction,&sep1,&end);

        const unsigned int l_command = err==1?(unsigned int)std::strlen(command):0;
        if (err==1 && l_command>=2 && command[l_command - 1]=='.') { // Selection shortcut.
          err = 4; sep0 = '['; sep1 = ']'; *restriction = '-';
          if (command[l_command - 2]!='.') { restriction[1] = '1'; command[l_command - 1] = 0; }
          else if (l_command>=3 && command[l_command - 3]!='.') { restriction[1] = '2'; command[l_command - 2] = 0; }
          else if (l_command>=4 && command[l_command - 4]!='.') { restriction[1] = '3'; command[l_command - 3] = 0; }
          restriction[2] = 0;
        }
        if (err==1) { // No selection -> all images.
          selection.assign(1,siz);
          cimg_forY(selection,y) selection[y] = (unsigned int)y;
        } else if (err==2 && sep0=='[' && item[std::strlen(command) + 1]==']') { // Empty selection.
          selection.assign(); is_restriction = true;
        } else if (err==4 && sep1==']') { // Other selections.
          is_restriction = true;
          if (!is_double_hyphen && (!std::strcmp("-wait",command) ||
                                    !std::strcmp("-cursor",command)))
            selection2cimg(restriction,10,CImgList<char>::empty(),command,true,
                           false,CImg<char>::empty()).move_to(selection);
          else if (!is_double_hyphen && command[1]=='i' && (!command[2] || !std::strcmp("-input",command)))
            selection2cimg(restriction,siz + 1,images_names,command,true,
                           true,new_name).move_to(selection);
          else if (!is_double_hyphen &&
                   ((command[1]=='e' && (!command[2] ||
                                         !std::strcmp("-echo",command) ||
                                         !std::strcmp("-error",command))) ||
                    !std::strcmp("-warn",command)))
            selection2cimg(restriction,callstack.size(),CImgList<char>::empty(),
                           command,true,false,CImg<char>::empty()).move_to(selection);
          else if (!is_double_hyphen && !std::strcmp("-pass",command))
            selection2cimg(restriction,parent_images.size(),parent_images_names,command,true,
                           false,CImg<char>::empty()).move_to(selection);
          else
            selection2cimg(restriction,siz,images_names,command,true,
                           false,CImg<char>::empty()).move_to(selection);
        } else {
          std::strncpy(command,item,_command.width() - 1);
          command[_command.width() - 1] = *restriction = 0;
        }
        if (is_double_hyphen) --item;
      } else {
        std::strncpy(command,item,_command.width() - 1);
        command[_command.width() - 1] = *restriction = 0;
      }
      position = position_argument;

      if (*command=='-') {
        std::strncpy(current_command,command,_current_command.width() - 1);
        current_command[_current_command.width() - 1] = 0;
      } else *current_command = 0;

      const bool
        is_verbose_command = *item=='-' && item[1]=='v' && (!item[2] || !std::strcmp(item,"-verbose")),
        is_echo_command = is_double_hyphen || is_verbose_command?false:
        *command=='-' && command[1]=='e' && (!command[2] || !std::strcmp(command,"-echo")),
        is_input_command = is_double_hyphen || is_verbose_command || is_echo_command?false:
        *command=='-' && command[1]=='i' && (!command[2] || !std::strcmp(command,"-input")),
        is_check_command = is_verbose_command || is_echo_command || is_input_command?false:!std::strcmp(item,"-check"),
        is_skip_command = is_verbose_command || is_echo_command || is_input_command || is_check_command?false:
        !std::strcmp(item,"-skip");

      // Check for verbosity command, prior to the first output of a log message.
      bool is_verbose = verbosity>=0 || is_debug, is_verbose_argument = false;
      const int old_verbosity = verbosity;
      if (is_verbose_command) {
        // Do a first fast check.
        if (*argument=='-' && !argument[1]) { --verbosity; is_verbose_argument = true; }
        else if (*argument=='+' && !argument[1]) { ++verbosity; is_verbose_argument = true; }
        else {
          gmic_substitute_args();
          if (*argument=='-' && !argument[1]) { --verbosity; is_verbose_argument = true; }
          else if (*argument=='+' && !argument[1]) { ++verbosity; is_verbose_argument = true; }
          else {
            float level = 0;
            if (cimg_sscanf(argument,"%f%c",&level,&end)==1) {
              verbosity = (int)cimg::round(level);
              is_verbose_argument = true;
            }
            else arg_error("verbose");
          }
        }
      }
      is_verbose = verbosity>=0 || is_debug;
      const bool is_very_verbose = verbosity>0 || is_debug;

      // Generate strings for displaying image selections when verbosity>=0.
      CImg<char> gmic_selection;
      if (is_verbose && !is_check_command && !is_skip_command && !is_echo_command && !is_verbose_command)
        selection2string(selection,images_names,1,gmic_selection);

      if (is_debug) {
        if (std::strcmp(item,initial_item))
          debug(images,"Item '%s' -> '%s', selection%s.",
                initial_item,item,gmic_selection.data());
        else
          debug(images,"Item '%s', selection%s.",
                initial_item,gmic_selection.data());
      }

      // Display starting message.
      if (is_start) {
        print(images,0,"Start G'MIC interpreter.");
        is_start = false;
      }

      // Cancellation point.
      if (*is_abort || is_abort_thread) {
        if (is_very_verbose) print(images,0,"Abort G'MIC interpreter.\n");
        dowhiles.assign();
        repeatdones.assign();
        position = commands_line.size();
        is_released = is_quit = true;
        break;
      }

      // Begin command interpretation.
      if (*item=='-' && item[1]) {

        // Manage command shortcuts.
        char command1 = command[1];
        const char
          command2 = command1?command[2]:0, command3 = command2?command[3]:0,
          command4 = command3?command[4]:0, command5 = command4?command[5]:0;

        static const char* onechar_shortcuts[] = {
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0-31
          0,0,0,0,0,"-mod","-and",0,0,0,"-mul","-add",0,"-sub",0,"-div",0,0,0,0,0,0,0,0,0,0,0,0, // 32-59
          "-lt","-set","-gt",0, // 60-63
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"-pow",0, // 64-95
          0,"-append","-blur","-cut","-display","-echo","-fill","-gradient",0,"-input","-image","-keep", // 96-107
          "-local","-command","-normalize","-output","-print","-quit","-resize","-split","-text","-status", // 108-117
          "-verbose","-window","-exec","-unroll","-crop",0,"-or",0,0,0 // 118-127
        };

        if (!command2) { // Single-char shortcut.
          const bool
            is_mquvx = command1=='m' || command1=='q' || command1=='u' || command1=='v' || command1=='x',
            is_deiopwx = command1=='d' || command1=='e' || command1=='i' || command1=='o' || command1=='p' ||
                         command1=='w' || command1=='x';
          if ((unsigned int)command1<128 && onechar_shortcuts[(unsigned int)command1] &&
              (!is_mquvx || (!is_double_hyphen && !is_restriction)) &&
              (!is_deiopwx || !is_double_hyphen)) {
            std::strcpy(command,onechar_shortcuts[(unsigned int)command1]);
            if (is_mquvx) { CImg<char>::string(command).move_to(_item); *command = 0; }
            else *item = 0;
          }

        } else if (!command3) { // Two-chars shortcuts.
          if (command1=='s' && command2=='h' && !is_double_hyphen) std::strcpy(command,"-shared");
          else if (command1=='m' && command2=='v') std::strcpy(command,"-move");
          else if (command1=='n' && command2=='m' && !is_double_hyphen) std::strcpy(command,"-name");
          else if (command1=='r' && command2=='m') std::strcpy(command,"-remove");
          else if (command1=='r' && command2=='v') std::strcpy(command,"-reverse");
          else if (command1=='<' && command2=='<') std::strcpy(command,"-bsl");
          else if (command1=='>' && command2=='>') std::strcpy(command,"-bsr");
          else if (command1=='=' && command2=='=') std::strcpy(command,"-eq");
          else if (command1=='>' && command2=='=') std::strcpy(command,"-ge");
          else if (command1=='<' && command2=='=') std::strcpy(command,"-le");
          else if (command1=='/' && command2=='/') std::strcpy(command,"-mdiv");
          else if (command1=='*' && command2=='*') std::strcpy(command,"-mmul");
          else if (command1=='!' && command2=='=') std::strcpy(command,"-neq");
          else if (command1=='_' && command2=='u' && !is_double_hyphen && !is_restriction) {
            CImg<char>::string("-_status").move_to(_item);
            *command = 0;
          }

        } else if (!command4 && command2=='3' && command3=='d') switch (command1) {
            // Three-chars shortcuts (ending with '3d').
          case 'd' : if (!is_double_hyphen) std::strcpy(command,"-display3d"); break;
          case 'j' : std::strcpy(command,"-object3d"); break;
          case '+' : std::strcpy(command,"-add3d"); break;
          case '/' : std::strcpy(command,"-div3d"); break;
          case 'f' : if (!is_double_hyphen && !is_restriction)
              CImg<char>::string("-focale3d").move_to(_item);
            break;
          case 'l' : if (!is_double_hyphen && !is_restriction)
              CImg<char>::string("-light3d").move_to(_item);
            break;
          case 'm' : if (!is_double_hyphen && !is_restriction)
              CImg<char>::string("-mode3d").move_to(_item);
          case '*' : std::strcpy(command,"-mul3d"); break;
          case 'o' : std::strcpy(command,"-opacity3d"); break;
          case 'p' : std::strcpy(command,"-primitives3d"); break;
          case 'r' : std::strcpy(command,"-rotate3d"); break;
          case 's' : std::strcpy(command,"-split3d"); break;
          case '-' : std::strcpy(command,"-sub3d"); break;
          case 't' : std::strcpy(command,"-texturize3d"); break;
          } else if (!command5 && command3=='3' && command4=='d') {
          // Four-chars shortcuts (ending with '3d').
          if (command1=='d' && command2=='b') {
            if (!is_double_hyphen && !is_restriction) CImg<char>::string("-double3d").move_to(_item);
          } else if (command1=='m' && command2=='d') {
            if (!is_double_hyphen && !is_restriction) CImg<char>::string("-moded3d").move_to(_item);
          }
          else if (command1=='r' && command2=='v') std::strcpy(command,"-reverse3d");
          else if (command1=='s' && command2=='l') {
            if (!is_double_hyphen && !is_restriction) CImg<char>::string("-specl3d").move_to(_item);
          }
          else if (command1=='s' && command2=='s') {
            if (!is_double_hyphen && !is_restriction) CImg<char>::string("-specs3d").move_to(_item);
          }
        }
        item = _item;
        command1 = *command?command[1]:item[1];

        // Check if a new name has been requested for a command that does not allow that.
        if (new_name && !is_double_hyphen && !is_input_command)
          error(images,0,0,
                "Item '%s %s': Unknow name '%s'.",
                initial_item,initial_argument,new_name.data());

        //----------------------------
        // Commands starting by '-_..'
        //----------------------------
        if (command1=='_') {

          // Status with escaped backslash.
          if (!std::strcmp("-_status",item)) {
            gmic_substitute_args();
            name.assign(2*std::strlen(argument) + 1);
            char *ptrd = name;
            for (const char *ptrs = argument; *ptrs; ++ptrs) {
              if (*ptrs=='\\') *(ptrd++) = '\\';
              *(ptrd++) = *ptrs;
            }
            *ptrd = 0;
            CImg<char>::string(name).move_to(status);
            _gmic_argument_text(status,name,is_verbose);
            print(images,0,"Set status to string '%s' (escaped backslash).",name.data());
            ++position; continue;
          }

        }

        //----------------------------
        // Commands starting by '-a..'
        //----------------------------
        else if (command1=='a') {

          // Append.
          if (!std::strcmp("-append",command)) {
            gmic_substitute_args();
            float align = 0;
            axis = sep = 0;
            if ((cimg_sscanf(argument,"%c%c",
                             &axis,&end)==1 ||
                 cimg_sscanf(argument,"%c,%f%c",
                             &axis,&align,&end)==2) &&
                (axis=='x' || axis=='y' || axis=='z' || axis=='c')) {
              print(images,0,"Append image%s along the '%c'-axis, with alignment %g.",
                    gmic_selection.data(),
                    axis,align);
              if (selection) {
                cimg_forY(selection,l) if (gmic_check(images[selection[l]]))
                  g_list.insert(gmic_check(images[selection[l]]),~0U,true);
                CImg<T> img = g_list.get_append(axis,align);
                name = images_names[selection[0]];
                if (is_double_hyphen) {
                  img.move_to(images);
                  images_names.insert(name.copymark());
                } else if (selection.height()>=2) {
                  remove_images(images,images_names,selection,1,selection.height() - 1);
                  img.move_to(images[selection[0]].assign());
                  name.move_to(images_names[selection[0]]);
                }
                g_list.assign();
              }
            } else if ((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c,%c%c",
                                    indices,&sep,&axis,&end)==3 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c,%c,%f%c",
                                    indices,&sep,&axis,&(align=0),&end)==4) &&
                       (axis=='x' || axis=='y' || axis=='z' || axis=='c') &&
                       sep==']' &&
                       (ind=selection2cimg(indices,images.size(),images_names,"-append",true,
                                           false,CImg<char>::empty())).height()==1) {
              print(images,0,"Append image [%u] to image%s, along the '%c'-axis, with alignment %g.",
                    *ind,gmic_selection.data(),axis,align);
              const CImg<T> img0 = gmic_image_arg(*ind);
              cimg_forY(selection,l) gmic_apply(append(img0,axis,align));
            } else arg_error("append");
            is_released = false; ++position; continue;
          }

          // Autocrop.
          if (!std::strcmp("-autocrop",command)) {
            gmic_substitute_args();
            if (*argument && cimg_sscanf(argument,"%4095[0-9.,eEinfa+-]%c",formula,&end)==1)
              try { CImg<T>(1).fill(argument,true,false).move_to(g_img); }
              catch (CImgException&) { g_img.assign(); }
            if (g_img) {
              print(images,0,"Auto-crop image%s by vector '%s'.",
                    gmic_selection.data(),
                    gmic_argument_text_printed());
              ++position;
            } else print(images,0,"Auto-crop image%s.",
                         gmic_selection.data());
            cimg_forY(selection,l) {
              if (g_img) {
                CImg<T>& img = images[selection[l]];
                g_img.assign(img.spectrum()).fill(argument,true,false);
                gmic_apply(gmic_autocrop(g_img));
              } else gmic_apply(gmic_autocrop());
            }
            g_img.assign();
            is_released = false; continue;
          }

          // Add.
          gmic_arithmetic_command("-add",
                                  operator+=,
                                  "Add %g%s to image%s",
                                  value,ssep,gmic_selection.data(),Tfloat,
                                  operator+=,
                                  "Add image [%d] to image%s",
                                  ind[0],gmic_selection.data(),
                                  "Add expression %s to image%s",
                                  gmic_argument_text_printed(),gmic_selection.data(),
                                  "Add image%s");

          // Add 3d objects together, or shift a 3d object.
          if (!std::strcmp("-add3d",command)) {
            gmic_substitute_args();
            float tx = 0, ty = 0, tz = 0;
            sep = 0;
            if (cimg_sscanf(argument,"%f%c",
                            &tx,&end)==1 ||
                cimg_sscanf(argument,"%f,%f%c",
                            &tx,&ty,&end)==2 ||
                cimg_sscanf(argument,"%f,%f,%f%c",
                            &tx,&ty,&tz,&end)==3) {
              print(images,0,"Shift 3d object%s by displacement (%g,%g,%g).",
                    gmic_selection.data(),
                    tx,ty,tz);
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                CImg<T>& img = images[uind];
                try { gmic_apply(shift_CImg3d(tx,ty,tz)); }
                catch (CImgException&) {
                  if (!img.is_CImg3d(true,&(*message=0)))
                    error(images,0,0,
                          "Command '-add3d': Invalid 3d object [%d], in selected image%s (%s).",
                          uind,gmic_selection_err.data(),message.data());
                  else throw;
                }
              }
              ++position;
            } else if (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep,&end)==2 &&
                       sep==']' &&
                       (ind=selection2cimg(indices,images.size(),images_names,"-add3d",true,
                                           false,CImg<char>::empty())).height()==1) {
              const CImg<T> img0 = gmic_image_arg(*ind);
              print(images,0,"Merge 3d object%s with 3d object [%u].",
                    gmic_selection.data(),*ind);
              cimg_forY(selection,l) {
                const unsigned int _ind = selection[l];
                CImg<T>& img = gmic_check(images[_ind]);
                g_list.assign(2);
                g_list[0].assign(img,true);
                g_list[1].assign(img0,true);
                CImg<T> res;
                try { CImg<T>::append_CImg3d(g_list).move_to(res); }
                catch (CImgException&) {
                  if (!img0.is_CImg3d(true,&(*message=0)))
                    error(images,0,0,
                          "Command '-add3d': Invalid 3d object [%u], in specified "
                          "argument '%s' (%s).",
                          *ind,gmic_argument_text(),message.data());
                  else if (!img.is_CImg3d(true,message))
                    error(images,0,0,
                          "Command '-add3d': Invalid 3d object [%d], in selected image%s (%s).",
                          _ind,gmic_selection_err.data(),message.data());
                  else throw;
                }
                if (is_double_hyphen) {
                  res.move_to(images);
                  images_names[_ind].get_copymark().move_to(images_names);
                } else res.move_to(images[_ind].assign());
              }
              g_list.assign();
              ++position;
            } else {
              print(images,0,"Merge 3d object%s.",
                    gmic_selection.data());
              if (selection) {
                g_list.assign(selection.height());
                cimg_forY(selection,l) g_list[l].assign(gmic_check(images[selection[l]]),true);
                CImg<T> img;
                try { CImg<T>::append_CImg3d(g_list).move_to(img); }
                catch (CImgException&) {
                  cimg_forY(selection,l) {
                    const unsigned int uind = selection[l];
                    if (!images[uind].is_CImg3d(true,&(*message=0)))
                      error(images,0,0,
                            "Command '-add3d': Invalid 3d object [%d], in selected image%s (%s).",
                            uind,gmic_selection_err.data(),message.data());
                  }
                  throw;
                }
                name = images_names[selection[0]];
                if (is_double_hyphen) {
                  img.move_to(images);
                  images_names.insert(name.copymark());
                } else if (selection.height()>=2) {
                  remove_images(images,images_names,selection,1,selection.height() - 1);
                  img.move_to(images[selection[0]].assign());
                  name.move_to(images_names[selection[0]]);
                }
                g_list.assign();
              }
            }
            is_released = false; continue;
          }

          // Absolute value.
          gmic_simple_command("-abs",abs,"Compute pointwise absolute value of image%s.");

          // Bitwise and.
          gmic_arithmetic_command("-and",
                                  operator&=,
                                  "Compute bitwise AND of image%s by %g%s",
                                  gmic_selection.data(),value,ssep,Tlong,
                                  operator&=,
                                  "Compute bitwise AND of image%s by image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute bitwise AND of image%s by expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute sequential bitwise AND of image%s");

          // Arc-tangent (two arguments).
          if (!std::strcmp("-atan2",command)) {
            gmic_substitute_args();
            sep = 0;
            if (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                            indices,&sep,&end)==2 && sep==']' &&
                (ind=selection2cimg(indices,images.size(),images_names,"-atan2",true,
                                    false,CImg<char>::empty())).height()==1) {
              print(images,0,"Compute pointwise oriented arc-tangent of image%s, "
                    "with x-argument [%u].",
                    gmic_selection.data(),
                    *ind);
              const CImg<T> img0 = gmic_image_arg(*ind);
              cimg_forY(selection,l) gmic_apply(atan2(img0));
            } else arg_error("atan2");
            is_released = false; ++position; continue;
          }

          // Arc-cosine.
          gmic_simple_command("-acos",acos,"Compute pointwise arc-cosine of image%s.");

          // Arc-sine.
          gmic_simple_command("-asin",asin,"Compute pointwise arc-sine of image%s.");

          // Arc-tangent.
          gmic_simple_command("-atan",atan,"Compute pointwise arc-tangent of image%s.");

        } // command1=='a'.

        //----------------------------
        // Commands starting by '-b..'
        //----------------------------
        else if (command1=='b') {

          // Blur.
          if (!std::strcmp("-blur",command)) {
            gmic_substitute_args();
            unsigned int is_gaussian = 0;
            float sigma = -1;
            sep = *argx = 0;
            boundary = 1;

            const char *_argument = argument;
            if (cimg_sscanf(argument,"%255[xyzc]%c",argx,&sep)==2 && sep==',') {
              _argument+=1 + std::strlen(argx);
            } else sep = *argx = 0;

            if ((cimg_sscanf(_argument,"%f%c",
                             &sigma,&end)==1 ||
                 (cimg_sscanf(_argument,"%f%c%c",
                              &sigma,&sep,&end)==2 && sep=='%') ||
                 cimg_sscanf(_argument,"%f,%u%c",
                             &sigma,&boundary,&end)==2 ||
                 (cimg_sscanf(_argument,"%f%c,%u%c",
                              &sigma,&sep,&boundary,&end)==3 && sep=='%') ||
                 cimg_sscanf(_argument,"%f,%u,%u%c",
                             &sigma,&boundary,&is_gaussian,&end)==3 ||
                 (cimg_sscanf(_argument,"%f%c,%u,%u%c",
                              &sigma,&sep,&boundary,&is_gaussian,&end)==4 && sep=='%')) &&
                sigma>=0 && boundary<=1 && is_gaussian<=1) {
              print(images,0,"Blur image%s%s%s%s with standard deviation %g%s, %s boundary conditions "
                    "and %s kernel.",
                    gmic_selection.data(),
                    *argx?" along the '":"",
                    *argx?argx:"",
                    *argx?std::strlen(argx)==1?"'-axis,":"'-axes,":"",
                    sigma,sep=='%'?"%":"",
                    boundary?"neumann":"dirichlet",
                    is_gaussian?"gaussian":"quasi-gaussian");
              if (sep=='%') sigma = -sigma;
              if (*argx) {
                g_img.assign(4,1,1,1,0);
                for (const char *s = argx; *s; ++s) g_img[*s>='x'?*s - 'x':3]+=sigma;
                cimg_forY(selection,l) gmic_apply(gmic_blur(g_img[0],g_img[1],g_img[2],g_img[3],
                                                            (bool)boundary,(bool)is_gaussian));
                g_img.assign();
              } else cimg_forY(selection,l) gmic_apply(blur(sigma,(bool)boundary,(bool)is_gaussian));
            } else arg_error("blur");
            is_released = false; ++position; continue;
          }

          // Box filter.
          if (!std::strcmp("-boxfilter",command)) {
            unsigned int order = 0;
            gmic_substitute_args();
            float sigma = -1;
            sep = *argx = 0;
            boundary = 1;
            const char *_argument = argument;
            if (cimg_sscanf(argument,"%255[xyzc]%c",argx,&sep)==2 && sep==',') {
              _argument+=1 + std::strlen(argx);
            } else sep = *argx = 0;
            if ((cimg_sscanf(_argument,"%f%c",
                             &sigma,&end)==1 ||
                 (cimg_sscanf(_argument,"%f%c%c",
                              &sigma,&sep,&end)==2 && sep=='%') ||
                 cimg_sscanf(_argument,"%f,%u%c",
                             &sigma,&order,&end)==2 ||
                 (cimg_sscanf(_argument,"%f%c,%u%c",
                              &sigma,&sep,&order,&end)==3 && sep=='%') ||
                 cimg_sscanf(_argument,"%f,%u,%u%c",
                             &sigma,&order,&boundary,&end)==3 ||
                 (cimg_sscanf(_argument,"%f%c,%u,%u%c",
                              &sigma,&sep,&order,&boundary,&end)==4 && sep=='%')) &&
                sigma>=0 && boundary<=1 && order<=2) {
              print(images,0,"Blur image%s%s%s%s with normalized box filter of size %g%s, order %u and "
                    "%s boundary conditions.",
                    gmic_selection.data(),
                    *argx?" along the '":"",
                    *argx?argx:"",
                    *argx?std::strlen(argx)==1?"'-axis,":"'-axes,":"",
                    sigma,sep=='%'?"%":"",
                    order,
                    boundary?"neumann":"dirichlet");
              if (sep=='%') sigma = -sigma;
              if (*argx) {
                g_img.assign(4,1,1,1,0);
                for (const char *s = argx; *s; ++s) g_img[*s>='x'?*s - 'x':3]+=sigma;
                cimg_forY(selection,l) gmic_apply(gmic_blur_box(g_img[0],g_img[1],g_img[2],g_img[3],
                                                                order,(bool)boundary));
                g_img.assign();
              } else cimg_forY(selection,l) gmic_apply(gmic_blur_box(sigma,order,(bool)boundary));
            } else arg_error("boxfilter");
            is_released = false; ++position; continue;
          }

          // Bitwise right shift.
          gmic_arithmetic_command("-bsr",
                                  operator>>=,
                                  "Compute bitwise right shift of image%s by %g%s",
                                  gmic_selection.data(),value,ssep,Tlong,
                                  operator>>=,
                                  "Compute bitwise right shift of image%s by image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute bitwise right shift of image%s by expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute sequential bitwise right shift of image%s");

          // Bitwise left shift.
          gmic_arithmetic_command("-bsl",
                                  operator<<=,
                                  "Compute bitwise left shift of image%s by %g%s",
                                  gmic_selection.data(),value,ssep,Tlong,
                                  operator<<=,
                                  "Compute bitwise left shift of image%s by image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute bitwise left shift of image%s by expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute sequential bitwise left shift of image%s");

          // Bilateral filter.
          if (!std::strcmp("-bilateral",command)) {
            gmic_substitute_args();
            float sigma_s = 0, sigma_r = 0, sampling_s = 0, sampling_r = 0;
            sep0 = sep1 = *argz = *argc = 0;
            if ((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             indices,argx,argy,&end)==3 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-],%f,%f%c",
                             indices,argx,argy,&sampling_s,&sampling_r,&end)==5) &&
                (cimg_sscanf(argx,"%f%c",&sigma_s,&end)==1 ||
                 (cimg_sscanf(argx,"%f%c%c",&sigma_s,&sep0,&end)==2 && sep0=='%')) &&
                (cimg_sscanf(argy,"%f%c",&sigma_r,&end)==1 ||
                 (cimg_sscanf(argy,"%f%c%c",&sigma_r,&sep1,&end)==2 && sep1=='%')) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-bilateral",true,
                                    false,CImg<char>::empty())).height()==1 &&
                sigma_s>=0 && sigma_r>=0 && sampling_s>=0 && sampling_r>=0) {
              print(images,0,"Apply joint bilateral filter on image%s, with guide image [%u], "
                    " standard deviations (%g%s,%g%s) and sampling (%g,%g).",
                    gmic_selection.data(),
                    *ind,
                    sigma_s,sep0=='%'?"%":"",
                    sigma_r,sep1=='%'?"%":"",
                    sampling_s,sampling_r);
              const CImg<T> guide = gmic_image_arg(*ind);
              if (sep0=='%') sigma_s = -sigma_s;
              if (sep1=='%') sigma_r = -sigma_r;
              cimg_forY(selection,l) gmic_apply(blur_bilateral(guide,sigma_s,sigma_r,sampling_s,sampling_r));
            } else if ((cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                                    argx,argy,&end)==2 ||
                        cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%f,%f%c",
                                    argx,argy,&sampling_s,&sampling_r,&end)==4) &&
                       (cimg_sscanf(argx,"%f%c",&sigma_s,&end)==1 ||
                        (cimg_sscanf(argx,"%f%c%c",&sigma_s,&sep0,&end)==2 && sep0=='%')) &&
                       (cimg_sscanf(argy,"%f%c",&sigma_r,&end)==1 ||
                        (cimg_sscanf(argy,"%f%c%c",&sigma_r,&sep1,&end)==2 && sep1=='%')) &&
                       sigma_s>=0 && sigma_r>=0 && sampling_s>=0 && sampling_r>=0) {
              print(images,0,"Apply bilateral filter on image%s, with standard deviations (%g%s,%g%s) and "
                    "sampling (%g,%g).",
                    gmic_selection.data(),
                    sigma_s,sep0=='%'?"%":"",
                    sigma_r,sep1=='%'?"%":"",
                    sampling_s,sampling_r);
              if (sep0=='%') sigma_s = -sigma_s;
              if (sep1=='%') sigma_r = -sigma_r;
              cimg_forY(selection,l)
                gmic_apply(blur_bilateral(images[selection[l]],sigma_s,sigma_r,sampling_s,sampling_r));
            } else arg_error("bilateral");
            is_released = false; ++position; continue;
          }

        } // command1=='b'.

        //----------------------------
        // Commands starting by '-c..'
        //----------------------------
        else if (command1=='c') {

          // Check expression or filename.
          if (is_check_command) {
            gmic_substitute_args();
            name.assign(argument,(unsigned int)std::strlen(argument) + 1);
            strreplace_fw(name);
            bool is_cond = false, is_filename = false;
            const CImg<T> &img = images.size()?images.back():CImg<T>::empty();
            try { if (img.eval(name,0,0,0,0,&images,&images)) is_cond = true; }
            catch (CImgException&) {
              is_filename = true;
              is_cond = check_filename(name);
            }
            if (is_very_verbose)
              print(images,0,"Check %s '%s' -> %s.",
                    is_filename?"file":"expression",
                    gmic_argument_text_printed(),
                    is_filename?(is_cond?"found":"not found"):(is_cond?"true":"false"));
            if (!is_cond) {
              if (is_first_item && callstack.size()>1 && callstack.back()[0]!='*')
                gmic::error(images,0,callstack.back(),"Command '-%s': Invalid argument '%s'.",
                            callstack.back().data(),_gmic_argument_text(parent_arguments,argument_text,true));
              else error(images,0,0,
                         "Command '-check': Expression '%s' is false (and no file with this name exists).",
                         gmic_argument_text());
            }
            ++position; continue;
          }

          // Crop.
          if (!std::strcmp("-crop",command)) {
            gmic_substitute_args();
            name.assign(64,8);
            char
              *const st0 = name.data(0,0), *const st1 = name.data(0,1), *const st2 = name.data(0,2),
              *const st3 = name.data(0,3), *const st4 = name.data(0,4), *const st5 = name.data(0,5),
              *const st6 = name.data(0,6), *const st7 = name.data(0,7);
            char sep2 = sep0 = sep1 = 0, sep3 = 0, sep4 = 0, sep5 = 0, sep6 = 0, sep7 = 0;
            float a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0, a7 = 0;
            *st0 = *st1 = *st2 = *st3 = *st4 = *st5 = *st6 = *st7 = 0;
            boundary = 0;
            if ((boundary=0,cimg_sscanf(argument,"%63[0-9.eE%+-],%63[0-9.eE%+-]%c",
                                        st0,
                                        st1,&end)==2 ||
                 cimg_sscanf(argument,"%63[0-9.eE%+-],%63[0-9.eE%+-],%u%c",
                             st0,
                             st1,&boundary,&end)==3) &&
                (cimg_sscanf(st0,"%f%c",&a0,&end)==1 ||
                 (cimg_sscanf(st0,"%f%c%c",&a0,&sep0,&end)==2 && sep0=='%')) &&
                (cimg_sscanf(st1,"%f%c",&a1,&end)==1 ||
                 (cimg_sscanf(st1,"%f%c%c",&a1,&sep1,&end)==2 && sep1=='%')) &&
                boundary<=1) {
              print(images,0,"Crop image%s with selection (%g%s) x (%g%s) and "
                    "%s boundary conditions.",
                    gmic_selection.data(),
                    a0,sep0=='%'?"%":"",
                    a1,sep1=='%'?"%":"",
                    boundary?"neumann":"dirichlet");
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                const int
                  x0 = (int)cimg::round(sep0=='%'?a0*(img.width() - 1)/100:a0),
                  x1 = (int)cimg::round(sep1=='%'?a1*(img.width() - 1)/100:a1);
                gmic_apply(crop(x0,x1,(bool)boundary));
              }
              ++position;
            } else if ((boundary=0,cimg_sscanf(argument,
                                               "%63[0-9.eE%+-],%63[0-9.eE%+-],"
                                               "%63[0-9.eE%+-],%63[0-9.eE%+-]%c",
                                               st0,st1,st2,st3,&end)==4 ||
                        cimg_sscanf(argument,
                                    "%63[0-9.eE%+-],%63[0-9.eE%+-],"
                                    "%63[0-9.eE%+-],%63[0-9.eE%+-],%u%c",
                                    st0,st1,st2,st3,&boundary,&end)==5) &&
                       (cimg_sscanf(st0,"%f%c",&a0,&end)==1 ||
                        (cimg_sscanf(st0,"%f%c%c",&a0,&sep0,&end)==2 && sep0=='%')) &&
                       (cimg_sscanf(st1,"%f%c",&a1,&end)==1 ||
                        (cimg_sscanf(st1,"%f%c%c",&a1,&sep1,&end)==2 && sep1=='%')) &&
                       (cimg_sscanf(st2,"%f%c",&a2,&end)==1 ||
                        (cimg_sscanf(st2,"%f%c%c",&a2,&sep2,&end)==2 && sep2=='%')) &&
                       (cimg_sscanf(st3,"%f%c",&a3,&end)==1 ||
                        (cimg_sscanf(st3,"%f%c%c",&a3,&sep3,&end)==2 && sep3=='%')) &&
                       boundary<=1) {
              print(images,0,
                    "Crop image%s with selection (%g%s,%g%s) x (%g%s,%g%s) and "
                    "%s boundary conditions.",
                    gmic_selection.data(),
                    a0,sep0=='%'?"%":"",
                    a1,sep1=='%'?"%":"",
                    a2,sep2=='%'?"%":"",
                    a3,sep3=='%'?"%":"",
                    boundary?"neumann":"dirichlet");
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                const int
                  x0 = (int)cimg::round(sep0=='%'?a0*(img.width() - 1)/100:a0),
                  y0 = (int)cimg::round(sep1=='%'?a1*(img.height() - 1)/100:a1),
                  x1 = (int)cimg::round(sep2=='%'?a2*(img.width() - 1)/100:a2),
                  y1 = (int)cimg::round(sep3=='%'?a3*(img.height() - 1)/100:a3);
                gmic_apply(crop(x0,y0,x1,y1,(bool)boundary));
              }
              ++position;
            } else if ((boundary=0,cimg_sscanf(argument,
                                               "%63[0-9.eE%+-],%63[0-9.eE%+-],%63[0-9.eE%+-],"
                                               "%63[0-9.eE%+-],%63[0-9.eE%+-],%63[0-9.eE%+-]%c",
                                               st0,st1,st2,st3,st4,st5,&end)==6 ||
                        cimg_sscanf(argument,"%63[0-9.eE%+-],%63[0-9.eE%+-],%63[0-9.eE%+-],"
                                    "%63[0-9.eE%+-],%63[0-9.eE%+-],%63[0-9.eE%+-],%u%c",
                                    st0,st1,st2,st3,st4,st5,&boundary,&end)==7) &&
                       (cimg_sscanf(st0,"%f%c",&a0,&end)==1 ||
                        (cimg_sscanf(st0,"%f%c%c",&a0,&sep0,&end)==2 && sep0=='%')) &&
                       (cimg_sscanf(st1,"%f%c",&a1,&end)==1 ||
                        (cimg_sscanf(st1,"%f%c%c",&a1,&sep1,&end)==2 && sep1=='%')) &&
                       (cimg_sscanf(st2,"%f%c",&a2,&end)==1 ||
                        (cimg_sscanf(st2,"%f%c%c",&a2,&sep2,&end)==2 && sep2=='%')) &&
                       (cimg_sscanf(st3,"%f%c",&a3,&end)==1 ||
                        (cimg_sscanf(st3,"%f%c%c",&a3,&sep3,&end)==2 && sep3=='%')) &&
                       (cimg_sscanf(st4,"%f%c",&a4,&end)==1 ||
                        (cimg_sscanf(st4,"%f%c%c",&a4,&sep4,&end)==2 && sep4=='%')) &&
                       (cimg_sscanf(st5,"%f%c",&a5,&end)==1 ||
                        (cimg_sscanf(st5,"%f%c%c",&a5,&sep5,&end)==2 && sep5=='%')) &&
                       boundary<=1) {
              print(images,0,"Crop image%s with selection (%g%s,%g%s,%g%s) x (%g%s,%g%s,%g%s) "
                    "and %s boundary conditions.",
                    gmic_selection.data(),
                    a0,sep0=='%'?"%":"",
                    a1,sep1=='%'?"%":"",
                    a2,sep2=='%'?"%":"",
                    a3,sep3=='%'?"%":"",
                    a4,sep4=='%'?"%":"",
                    a5,sep5=='%'?"%":"",
                    boundary?"neumann":"dirichlet");
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                const int
                  x0 = (int)cimg::round(sep0=='%'?a0*(img.width() - 1)/100:a0),
                  y0 = (int)cimg::round(sep1=='%'?a1*(img.height() - 1)/100:a1),
                  z0 = (int)cimg::round(sep2=='%'?a2*(img.depth() - 1)/100:a2),
                  x1 = (int)cimg::round(sep3=='%'?a3*(img.width() - 1)/100:a3),
                  y1 = (int)cimg::round(sep4=='%'?a4*(img.height() - 1)/100:a4),
                  z1 = (int)cimg::round(sep5=='%'?a5*(img.depth() - 1)/100:a5);
                gmic_apply(crop(x0,y0,z0,x1,y1,z1,(bool)boundary));
              }
              ++position;
            } else if ((boundary=0,cimg_sscanf(argument,
                                               "%63[0-9.eE%+-],%63[0-9.eE%+-],%63[0-9.eE%+-],"
                                               "%63[0-9.eE%+-],%63[0-9.eE%+-],%63[0-9.eE%+-],"
                                               "%63[0-9.eE%+-],%63[0-9.eE%+-]%c",
                                               st0,st1,st2,st3,st4,st5,st6,st7,&end)==8 ||
                        cimg_sscanf(argument,"%63[0-9.eE%+-],%63[0-9.eE%+-],%63[0-9.eE%+-],"
                                    "%63[0-9.eE%+-],%63[0-9.eE%+-],%63[0-9.eE%+-],"
                                    "%63[0-9.eE%+-],%63[0-9.eE%+-],%u%c",
                                    st0,st1,st2,st3,st4,st5,st6,st7,&boundary,&end)==9) &&
                       (cimg_sscanf(st0,"%f%c",&a0,&end)==1 ||
                        (cimg_sscanf(st0,"%f%c%c",&a0,&sep0,&end)==2 && sep0=='%')) &&
                       (cimg_sscanf(st1,"%f%c",&a1,&end)==1 ||
                        (cimg_sscanf(st1,"%f%c%c",&a1,&sep1,&end)==2 && sep1=='%')) &&
                       (cimg_sscanf(st2,"%f%c",&a2,&end)==1 ||
                        (cimg_sscanf(st2,"%f%c%c",&a2,&sep2,&end)==2 && sep2=='%')) &&
                       (cimg_sscanf(st3,"%f%c",&a3,&end)==1 ||
                        (cimg_sscanf(st3,"%f%c%c",&a3,&sep3,&end)==2 && sep3=='%')) &&
                       (cimg_sscanf(st4,"%f%c",&a4,&end)==1 ||
                        (cimg_sscanf(st4,"%f%c%c",&a4,&sep4,&end)==2 && sep4=='%')) &&
                       (cimg_sscanf(st5,"%f%c",&a5,&end)==1 ||
                        (cimg_sscanf(st5,"%f%c%c",&a5,&sep5,&end)==2 && sep5=='%')) &&
                       (cimg_sscanf(st6,"%f%c",&a6,&end)==1 ||
                        (cimg_sscanf(st6,"%f%c%c",&a6,&sep6,&end)==2 && sep6=='%')) &&
                       (cimg_sscanf(st7,"%f%c",&a7,&end)==1 ||
                        (cimg_sscanf(st7,"%f%c%c",&a7,&sep7,&end)==2 && sep7=='%')) &&
                       boundary<=1) {
              print(images,0,
                    "Crop image%s with selection (%g%s,%g%s,%g%s,%g%s) x (%g%s,%g%s,%g%s,%g%s) "
                    "and %s boundary conditions.",
                    gmic_selection.data(),
                    a0,sep0=='%'?"%":"",
                    a1,sep1=='%'?"%":"",
                    a2,sep2=='%'?"%":"",
                    a3,sep3=='%'?"%":"",
                    a4,sep4=='%'?"%":"",
                    a5,sep5=='%'?"%":"",
                    a6,sep6=='%'?"%":"",
                    a7,sep7=='%'?"%":"",
                    boundary?"neumann":"dirichlet");
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                const int
                  x0 = (int)cimg::round(sep0=='%'?a0*(img.width() - 1)/100:a0),
                  y0 = (int)cimg::round(sep1=='%'?a1*(img.height() - 1)/100:a1),
                  z0 = (int)cimg::round(sep2=='%'?a2*(img.depth() - 1)/100:a2),
                  v0 = (int)cimg::round(sep3=='%'?a3*(img.spectrum() - 1)/100:a3),
                  x1 = (int)cimg::round(sep4=='%'?a4*(img.width() - 1)/100:a4),
                  y1 = (int)cimg::round(sep5=='%'?a5*(img.height() - 1)/100:a5),
                  z1 = (int)cimg::round(sep6=='%'?a6*(img.depth() - 1)/100:a6),
                  v1 = (int)cimg::round(sep7=='%'?a7*(img.spectrum() - 1)/100:a7);
                gmic_apply(crop(x0,y0,z0,v0,x1,y1,z1,v1,(bool)boundary));
              }
              ++position;
            } else {
#if cimg_display==0
              print(images,0,"Crop image%s in interactive mode (skipped, no display support).",
                    gmic_selection.data());
#else // #if cimg_display==0
              bool is_available_display = false;
              try {
                is_available_display = (bool)CImgDisplay::screen_width();
              } catch (CImgDisplayException&) {
                print(images,0,"Crop image%s in interactive mode (skipped, no display available).",
                      gmic_selection.data());
              }
              if (is_available_display) {
                print(images,0,"Crop image%s in interactive mode.",
                      gmic_selection.data());
                CImgDisplay _disp, &disp = _display_window[0]?_display_window[0]:_disp;
                cimg_forY(selection,l) {
                  CImg<T>& img = gmic_check(images[selection[l]]);
                  if (disp) disp.resize(cimg_fitscreen(img.width(),img.height(),1),false);
                  else disp.assign(cimg_fitscreen(img.width(),img.height(),1),0,1);
                  disp.set_title("%s: crop",basename(images_names[selection[l]].data()));
                  const CImg<int> s = img.get_select(disp,2);
                  print(images,0,"Crop image [%d] with selection (%d,%d,%d) x (%d,%d,%d).",
                        selection[l],
                        s[0],s[1],s[2],
                        s[3],s[4],s[5]);
                  gmic_apply(crop(s[0],s[1],s[2],s[3],s[4],s[5]));
                }
              }
#endif // #if cimg_display==0
            }
            is_released = false; continue;
          }

          // Cut.
          if (!std::strcmp("-cut",command)) {
            gmic_substitute_args();
            ind0.assign(); ind1.assign();
            sep0 = sep1 = *argx = *argy = 0;
            value0 = value1 = 0;
            if (cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-]%c",
                            argx,argy,&end)==2 &&
                ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep0,&end)==2 &&
                  sep0==']' &&
                  (ind0=selection2cimg(indices,images.size(),images_names,"-cut",true,
                                       false,CImg<char>::empty())).height()==1) ||
                 (cimg_sscanf(argx,"%lf%c%c",&value0,&sep0,&end)==2 && sep0=='%') ||
                 cimg_sscanf(argx,"%lf%c",&value0,&end)==1) &&
                ((cimg_sscanf(argy,"[%255[a-zA-Z0-9_.%+-]%c%c",formula,&sep1,&end)==2 &&
                  sep1==']' &&
                  (ind1=selection2cimg(formula,images.size(),images_names,"-cut",true,
                                       false,CImg<char>::empty())).height()==1) ||
                 (cimg_sscanf(argy,"%lf%c%c",&value1,&sep1,&end)==2 && sep1=='%') ||
                 cimg_sscanf(argy,"%lf%c",&value1,&end)==1)) {
              if (ind0) { value0 = images[*ind0].min(); sep0 = 0; }
              if (ind1) { value1 = images[*ind1].max(); sep1 = 0; }
              print(images,0,"Cut image%s in range [%g%s,%g%s].",
                    gmic_selection.data(),
                    value0,sep0=='%'?"%":"",
                    value1,sep1=='%'?"%":"");
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                nvalue0 = value0; nvalue1 = value1;
                vmin = vmax = 0;
                if (sep0=='%' || sep1=='%') {
                  if (img) vmax = (double)img.max_min(vmin);
                  if (sep0=='%') nvalue0 = vmin + (vmax - vmin)*value0/100;
                  if (sep1=='%') nvalue1 = vmin + (vmax - vmin)*value1/100;
                }
                gmic_apply(cut((T)nvalue0,(T)nvalue1));
              }
              ++position;
            } else if (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep0,&end)==2 &&
                       sep0==']' &&
                       (ind0=selection2cimg(indices,images.size(),images_names,"-cut",true,
                                            false,CImg<char>::empty())).height()==1) {
              if (images[*ind0]) value1 = (double)images[*ind0].max_min(value0);
              print(images,0,"Cut image%s in range [%g,%g].",
                    gmic_selection.data(),
                    value0,
                    value1);
              cimg_forY(selection,l) gmic_apply(cut((T)value0,(T)value1));
              ++position;
            } else {
              g_img.assign(2,selection.height()).fill(0);
#if cimg_display==0
              print(images,0,"Cut image%s in interactive mode (skipped, no display support).",
                    gmic_selection.data());
#else // #if cimg_display==0
              bool is_available_display = false;
              try {
                is_available_display = (bool)CImgDisplay::screen_width();
              } catch (CImgDisplayException&) {
                print(images,0,"Cut image%s in interactive mode (skipped, no display available).",
                      gmic_selection.data());
              }
              if (is_available_display) {
                print(images,0,"Cut image%s in interactive mode.",
                      gmic_selection.data());
                CImgDisplay _disp, &disp = _display_window[0]?_display_window[0]:_disp;
                cimg_forY(selection,l) {
                  CImg<T> &img = gmic_check(images[selection[l]]);
                  value0 = value1 = 0;
                  if (img) {
                    CImg<T> visu = img.depth()>1?img.get_projections2d(img.width()/2,
                                                                       img.height()/2,
                                                                       img.depth()/2).
                      channels(0,cimg::min(3,img.spectrum()) - 1):
                      img.get_channels(0,cimg::min(3,img.spectrum() - 1));
                    const unsigned int
                      w = CImgDisplay::_fitscreen(visu.width(),visu.height(),1,256,-85,false),
                      h = CImgDisplay::_fitscreen(visu.width(),visu.height(),1,256,-85,true);
                    if (disp) disp.resize(w,h,false); else disp.assign(w,h,0,0);
                    double percent0 = 0, percent1 = 100;
                    vmin = 0, vmax = (double)img.max_min(vmin);
                    bool stopflag = false, is_clicked = false;
                    int omx = -1, omy = -1;
                    g_img_uc.assign();
                    for (disp.show().flush(); !stopflag; ) {
                      static const unsigned char white[] = { 255,255,255 }, black[] = { 0,0,0 };
                      const unsigned int key = disp.key();
                      if (!g_img_uc) {
                        value0 = vmin + percent0*(vmax - vmin)/100;
                        value1 = vmin + percent1*(vmax - vmin)/100;
                        disp.display((g_img_uc=visu.get_cut((T)value0,(T)value1).normalize((T)0,(T)255).resize(disp)).
                                     draw_text(0,0,"Cut [%g,%g] = [%.3g%%,%.3g%%]",
                                               white,black,0.7f,13,value0,value1,percent0,percent1)).
                          set_title("%s (%dx%dx%dx%d)",
                                    basename(images_names[selection[l]].data()),
                                    img.width(),img.height(),img.depth(),img.spectrum()).wait();
                      }
                      const int mx = disp.mouse_x(), my = disp.mouse_y();
                      if (disp.button()) {
                        if (mx>=0 && my>=0 && (mx!=omx || my!=omy)) {
                          percent0 = (my - 16)*100.0/(disp.height() - 32);
                          percent1 = (mx - 16)*100.0/(disp.width() - 32);
                          if (percent0<0) percent0 = 0; else if (percent0>101) percent0 = 101;
                          if (percent1<0) percent1 = 0; else if (percent1>101) percent1 = 101;
                          if (percent0>percent1) cimg::swap(percent0,percent1);
                          omx = mx; omy = my; g_img_uc.assign();
                        }
                        is_clicked = true;
                      } else if (is_clicked) break;
                      if (disp.is_closed() || (key && key!=cimg::keyCTRLLEFT)) stopflag = true;
                      if (key==cimg::keyD && disp.is_keyCTRLLEFT()) {
                        disp.resize(cimg_fitscreen(3*disp.width()/2,3*disp.height()/2,1),
                                    stopflag=false).set_key(cimg::keyD,false);
                        g_img_uc.assign();
                      }
                      if (key==cimg::keyC && disp.is_keyCTRLLEFT()) {
                        disp.resize(cimg_fitscreen(2*disp.width()/3,2*disp.height()/3,1),
                                    stopflag=false).set_key(cimg::keyC,false);
                        g_img_uc.assign();
                      }
                      if (disp.is_resized()) { disp.resize(false); g_img_uc.assign(); }
                    }
                    value0 = vmin + percent0*(vmax - vmin)/100;
                    value1 = vmin + percent1*(vmax - vmin)/100;
                    print(images,0,"Cut image [%d] in range [%g,%g] = [%.3g%%,%.3g%%].",
                          selection[l],value0,value1,percent0,percent1);
                    gmic_apply(cut((T)value0,(T)value1));
                  } else gmic_apply(replace(img));
                  g_img(0,l) = (T)value0; g_img(1,l) = (T)value1;
                }
                g_img_uc.assign();
              }
#endif // #if cimg_display==0
              g_img.value_string().move_to(status);
              g_img.assign();
            }
            is_released = false; continue;
          }

          // Keep channels.
          if (!std::strcmp("-channels",command)) {
            gmic_substitute_args();
            ind0.assign(); ind1.assign();
            value0 = value1 = 0;
            sep0 = sep1 = *argx = *argy = 0;
            if (cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-]%c",
                            argx,&end)==1 &&
                ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c]",indices,&sep0,&end)==2 &&
                  sep0==']' &&
                  (ind0=selection2cimg(indices,images.size(),images_names,"-channels",true,
                                       false,CImg<char>::empty())).height()==1) ||
                 cimg_sscanf(argx,"%lf%c",&value0,&end)==1 ||
                 (cimg_sscanf(argx,"%lf%c%c",&value0,&sep0,&end)==2 && sep0=='%'))) {
              if (ind0) { value0 = images[*ind0].spectrum() - 1.; sep0 = 0; }
              print(images,0,"Keep channel %g%s of image%s.",
                    value0,sep0=='%'?"%":"",
                    gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                nvalue0 = cimg::round(sep0=='%'?value0*(img.spectrum() - 1)/100:value0);
                gmic_apply(channel((int)nvalue0));
              }
            } else if (cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-]%c",
                                   argx,argy,&end)==2 &&
                       ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep0,&end)==2 &&
                         sep0==']' &&
                         (ind0=selection2cimg(indices,images.size(),images_names,"-channels",true,
                                              false,CImg<char>::empty())).height()==1) ||
                        cimg_sscanf(argx,"%lf%c",&value0,&end)==1 ||
                        (cimg_sscanf(argx,"%lf%c%c",&value0,&sep0,&end)==2 && sep0=='%')) &&
                       ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c",formula,&sep0,&end)==2 &&
                         sep0==']' &&
                         (ind1=selection2cimg(formula,images.size(),images_names,"-channels",true,
                                              false,CImg<char>::empty())).height()==1) ||
                        cimg_sscanf(argy,"%lf%c",&value1,&end)==1 ||
                        (cimg_sscanf(argy,"%lf%c%c",&value1,&sep1,&end)==2 && sep1=='%'))) {
              if (ind0) { value0 = images[*ind0].spectrum() - 1.; sep0 = 0; }
              if (ind1) { value1 = images[*ind1].spectrum() - 1.; sep1 = 0; }
              print(images,0,"Keep channels %g%s...%g%s of image%s.",
                    value0,sep0=='%'?"%":"",
                    value1,sep1=='%'?"%":"",
                    gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                nvalue0 = cimg::round(sep0=='%'?value0*(img.spectrum() - 1)/100:value0);
                nvalue1 = cimg::round(sep1=='%'?value1*(img.spectrum() - 1)/100:value1);
                gmic_apply(channels((int)nvalue0,(int)nvalue1));
              }
            } else arg_error("channels");
            is_released = false; ++position; continue;
          }

          // Keep columns.
          if (!std::strcmp("-columns",command)) {
            gmic_substitute_args();
            ind0.assign(); ind1.assign();
            value0 = value1 = 0;
            sep0 = sep1 = *argx = *argy = 0;
            if (cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-]%c",
                            argx,&end)==1 &&
                ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c]",indices,&sep0,&end)==2 &&
                  sep0==']' &&
                  (ind0=selection2cimg(indices,images.size(),images_names,"-columns",true,
                                       false,CImg<char>::empty())).height()==1) ||
                 cimg_sscanf(argx,"%lf%c",&value0,&end)==1 ||
                 (cimg_sscanf(argx,"%lf%c%c",&value0,&sep0,&end)==2 && sep0=='%'))) {
              if (ind0) { value0 = images[*ind0].width() - 1.; sep0 = 0; }
              print(images,0,"Keep column %g%s of image%s.",
                    value0,sep0=='%'?"%":"",
                    gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                nvalue0 = cimg::round(sep0=='%'?value0*(img.width() - 1)/100:value0);
                gmic_apply(column((int)nvalue0));
              }
            } else if (cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-]%c",
                                   argx,argy,&end)==2 &&
                       ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep0,&end)==2 &&
                         sep0==']' &&
                         (ind0=selection2cimg(indices,images.size(),images_names,"-columns",true,
                                              false,CImg<char>::empty())).height()==1) ||
                        cimg_sscanf(argx,"%lf%c",&value0,&end)==1 ||
                        (cimg_sscanf(argx,"%lf%c%c",&value0,&sep0,&end)==2 && sep0=='%')) &&
                       ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c",formula,&sep0,&end)==2 &&
                         sep0==']' &&
                         (ind1=selection2cimg(formula,images.size(),images_names,"-columns",true,
                                              false,CImg<char>::empty())).height()==1) ||
                        cimg_sscanf(argy,"%lf%c",&value1,&end)==1 ||
                        (cimg_sscanf(argy,"%lf%c%c",&value1,&sep1,&end)==2 && sep1=='%'))) {
              if (ind0) { value0 = images[*ind0].width() - 1.; sep0 = 0; }
              if (ind1) { value1 = images[*ind1].width() - 1.; sep1 = 0; }
              print(images,0,"Keep columns %g%s...%g%s of image%s.",
                    value0,sep0=='%'?"%":"",
                    value1,sep1=='%'?"%":"",
                    gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                nvalue0 = cimg::round(sep0=='%'?value0*(img.width() - 1)/100:value0);
                nvalue1 = cimg::round(sep1=='%'?value1*(img.width() - 1)/100:value1);
                gmic_apply(columns((int)nvalue0,(int)nvalue1));
              }
            } else arg_error("columns");
            is_released = false; ++position; continue;
          }

          // Import custom commands.
          if (!std::strcmp("-command",item)) {
            gmic_substitute_args();
            name.assign(argument,(unsigned int)std::strlen(argument) + 1);
            const char *arg_command_text = gmic_argument_text_printed();
            unsigned int offset_argument_text = 0;
            char *arg_command = name;
            strreplace_fw(arg_command);

            unsigned int siz = 0;
            for (unsigned int l = 0; l<512; ++l) siz+=commands[l].size();

            bool add_debug_info = true;
            if ((*arg_command=='0' || *arg_command=='1') && arg_command[1]==',') {
              add_debug_info = (*arg_command=='1');
              arg_command+=2; arg_command_text+=2; offset_argument_text = 2;
            }

            std::FILE *file = std::fopen(arg_command,"rb");
            if (file) {
              print(images,0,"Import custom commands from file '%s'%s",
                    arg_command_text,
                    !add_debug_info?" without debug info":"");
              add_commands(file,add_debug_info?arg_command:0);
              std::fclose(file);
            } else if (!cimg::strncasecmp(arg_command,"http://",7) ||
                       !cimg::strncasecmp(arg_command,"https://",8)) { // Try to read from network.
              print(images,0,"Import custom commands from URL '%s'%s",
                    arg_command_text,
                    !add_debug_info?" without debug info":"");
              try {
                file = std::fopen(cimg::load_network(arg_command,argx),"r");
              } catch (...) {
                file = 0;
              }
              if (file) {
                CImg<char> _status;
                status.move_to(_status); // Save status because 'add_commands' can change it.
                const int _verbosity = verbosity;
                const bool _is_debug = is_debug;
                verbosity = -1; is_debug = false;
                try {
                  add_commands(file,add_debug_info?arg_command:0);
                  std::fclose(file);
                } catch (...) {
                  std::fclose(file);
                  file = 0;
                }
                verbosity = _verbosity; is_debug = _is_debug;
                _status.move_to(status);
              }
              if (!file)
                error(images,0,0,
                      "Command '-command': Unable to load valid custom command file '%s' "
                      "from network.",
                      gmic_argument_text() + offset_argument_text);
              std::remove(argx);
            } else {
              print(images,0,"Import custom commands from expression '%s'",
                    arg_command_text);
              add_commands(arg_command);
            }
            if (is_verbose) {
              unsigned int nb_added = 0;
              for (unsigned int l = 0; l<512; ++l) nb_added+=commands[l].size();
              nb_added-=siz;
              cimg::mutex(29);
              std::fprintf(cimg::output()," (added %u command%s, total %u).",
                           nb_added,nb_added>1?"s":"",siz + nb_added);
              std::fflush(cimg::output());
              cimg::mutex(29,0);
            }
            ++position; continue;
          }

          // Check validity of 3d object.
          if (!is_double_hyphen && !std::strcmp("-check3d",command)) {
            gmic_substitute_args();
            bool is_full_check = true;
            if (!argument[1] && (*argument=='0' || *argument=='1')) {
              is_full_check = (*argument=='1');
              ++position;
            } else is_full_check = true;
            if (is_very_verbose)
              print(images,0,"Check validity of 3d object%s (%s check)",
                    gmic_selection.data(),
                    is_full_check?"full":"fast");
            cimg_forY(selection,l) {
              const unsigned int uind = selection[l];
              CImg<T>& img = gmic_check(images[uind]);
              if (!img.is_CImg3d(is_full_check,&(*message=0))) {
                if (is_very_verbose) {
                  cimg::mutex(29);
                  std::fprintf(cimg::output()," -> invalid.");
                  std::fflush(cimg::output());
                  cimg::mutex(29,0);
                }
                error(images,0,0,
                      "Command '-check3d': Invalid 3d object [%d], in selected image%s (%s).",
                      uind,gmic_selection_err.data(),message.data());
              }
            }
            if (is_very_verbose) {
              cimg::mutex(29);
              std::fprintf(cimg::output()," -> valid.");
              std::fflush(cimg::output());
              cimg::mutex(29,0);
            }
            continue;
          }

          // Cosine.
          gmic_simple_command("-cos",cos,"Compute pointwise cosine of image%s.");

          // Convolve.
          if (!std::strcmp("-convolve",command)) {
            gmic_substitute_args();
            unsigned int is_normalized = 0;
            boundary = 1;
            sep = 0;
            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                              indices,&sep,&end)==2 && sep==']') ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u%c",
                             indices,&boundary,&end)==2 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u,%u%c",
                             indices,&boundary,&is_normalized,&end)==3) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-convolve",true,
                                    false,CImg<char>::empty())).height()==1 &&
                boundary<=1) {
              print(images,0,
                    "Convolve image%s with mask [%u] and %s boundary conditions, "
                    "with%s normalization.",
                    gmic_selection.data(),
                    *ind,
                    boundary?"neumann":"dirichlet",
                    is_normalized?"":"out");
              const CImg<T> mask = gmic_image_arg(*ind);
              cimg_forY(selection,l) gmic_apply(convolve(mask,boundary,(bool)is_normalized));
            } else arg_error("convolve");
            is_released = false; ++position; continue;
          }

          // Correlate.
          if (!std::strcmp("-correlate",command)) {
            gmic_substitute_args();
            unsigned int is_normalized = 0;
            boundary = 1;
            sep = 0;
            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                              indices,&sep,&end)==2 && sep==']') ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u%c",
                             indices,&boundary,&end)==2 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u,%u%c",
                             indices,&boundary,&is_normalized,&end)==3) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-correlate",true,
                                    false,CImg<char>::empty())).height()==1 &&
                boundary<=1) {
              print(images,0,
                    "Correlate image%s with mask [%u] and %s boundary conditions, "
                    "with%s normalization.",
                    gmic_selection.data(),
                    *ind,
                    boundary?"neumann":"dirichlet",
                    is_normalized?"":"out");
              const CImg<T> mask = gmic_image_arg(*ind);
              cimg_forY(selection,l) gmic_apply(correlate(mask,boundary,(bool)is_normalized));
            } else arg_error("correlate");
            is_released = false; ++position; continue;
          }

          // Set 3d object color.
          if (!std::strcmp("-color3d",command) || !std::strcmp("-col3d",command)) {
            gmic_substitute_args();
            float R = 200, G = 200, B = 200;
            opacity = -1;
            if ((cimg_sscanf(argument,"%f%c",
                             &R,&end)==1 && ((B=G=R),1)) ||
                (cimg_sscanf(argument,"%f,%f%c",
                             &R,&G,&end)==2 && ((B=0),1)) ||
                cimg_sscanf(argument,"%f,%f,%f%c",
                            &R,&G,&B,&end)==3 ||
                cimg_sscanf(argument,"%f,%f,%f,%f%c",
                            &R,&G,&B,&opacity,&end)==4) {
              const bool set_opacity = (opacity>=0);
              if (set_opacity)
                print(images,0,"Set colors of 3d object%s to (%g,%g,%g), with opacity %g.",
                      gmic_selection.data(),
                      R,G,B,
                      opacity);
              else
                print(images,0,"Set color of 3d object%s to (%g,%g,%g).",
                      gmic_selection.data(),
                      R,G,B);
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                CImg<T>& img = gmic_check(images[uind]);
                try { gmic_apply(color_CImg3d(R,G,B,opacity,true,set_opacity)); }
                catch (CImgException&) {
                  if (!img.is_CImg3d(true,&(*message=0)))
                    error(images,0,0,
                          "Command '-color3d': Invalid 3d object [%d], "
                          "in selected image%s (%s).",
                          uind,gmic_selection_err.data(),message.data());
                  else throw;
                }
              }
            } else arg_error("color3d");
            is_released = false; ++position; continue;
          }

          // Cumulate.
          if (!std::strcmp("-cumulate",command)) {
            gmic_substitute_args();
            bool is_axes_argument = true;
            for (const char *s = argument; *s; ++s) {
              const char _s = *s;
              if (_s!='x' && _s!='y' && _s!='z' && _s!='c') { is_axes_argument = false; break; }
            }
            if (*argument && is_axes_argument) {
              print(images,0,"Cumulate values of image%s along the '%s'-ax%cs.",
                    gmic_selection.data(),
                    gmic_argument_text_printed(),
                    std::strlen(argument)>1?'e':'i');
              cimg_forY(selection,l) gmic_apply(cumulate(argument));
              ++position;
            } else {
              print(images,0,"Cumulate values of image%s.",
                    gmic_selection.data());
              cimg_forY(selection,l) gmic_apply(cumulate());
            }
            is_released = false; continue;
          }

          // Hyperbolic cosine.
          gmic_simple_command("-cosh",cosh,"Compute pointwise hyperbolic cosine of image%s.");

          // Camera input.
          if (!std::strcmp("-camera",item)) {
            gmic_substitute_args();
            float
              cam_index = 0, nb_frames = 1, skip_frames = 0,
              capture_width = 0, capture_height = 0;
            if ((cimg_sscanf(argument,"%f%c",
                             &cam_index,&end)==1 ||
                 cimg_sscanf(argument,"%f,%f%c",
                             &cam_index,&nb_frames,&end)==2 ||
                 cimg_sscanf(argument,"%f,%f,%f%c",
                             &cam_index,&nb_frames,&skip_frames,&end)==3 ||
                 cimg_sscanf(argument,"%f,%f,%f,%f,%f%c",
                             &cam_index,&nb_frames,&skip_frames,
                             &capture_width,&capture_height,&end)==5) &&
                cam_index>=0 && nb_frames>=0 && skip_frames>=0 &&
                ((!capture_width && !capture_height) || (capture_width>0 && capture_height>0)))
              ++position;
            cam_index = cimg::round(cam_index);
            nb_frames = cimg::round(nb_frames);
            skip_frames = cimg::round(skip_frames);
            capture_width = cimg::round(capture_width);
            capture_height = cimg::round(capture_height);
            if (!nb_frames) {
              print(images,0,"Release camera #%g.",cam_index);
              CImg<T>::get_load_camera((unsigned int)cam_index,0,true);
            } else {
              if (capture_width)
                print(images,0,"Insert %g image%s from camera #%g, with %g frames skipping "
                      "and resolution %gx%g.",
                      cam_index,nb_frames,nb_frames>1?"s":"",skip_frames,
                      capture_width,capture_height);
              else print(images,0,"Insert %g image%s from camera #%g, with %g frames skipping.",
                         cam_index,nb_frames,nb_frames>1?"s":"",skip_frames);
              cimg_snprintf(title,_title.width(),"[Camera #%g]",cam_index);
              CImg<char>::string(title).move_to(name);
              if (nb_frames>1) {
                cimg::mutex(29);
                std::fputc('\n',cimg::output());
                std::fflush(cimg::output());
                cimg::mutex(29,0);
              }
              for (unsigned int k = 0; k<(unsigned int)nb_frames; ++k) {
                if (nb_frames>1 && is_verbose) {
                  cimg::mutex(29);
                  std::fprintf(cimg::output(),"\r  > Image %u/%u        ",
                               k + 1,(unsigned int)nb_frames);
                  std::fflush(cimg::output());
                  cimg::mutex(29,0);
                }
                CImg<T>::get_load_camera((unsigned int)cam_index,(unsigned int)skip_frames,false,
                                         (unsigned int)capture_width,(unsigned int)capture_height).
                  move_to(images);
                images_names.insert(name);
              }
            }
            is_released = false; continue;
          }

          // Show/hide mouse cursor.
          if (!is_double_hyphen && !std::strcmp("-cursor",command)) {
            gmic_substitute_args();
            if (!is_restriction)
              CImg<unsigned int>::vector(0,1,2,3,4,5,6,7,8,9).move_to(selection);
            bool state = true;
            if (!argument[1] && (*argument=='0' || *argument=='1')) {
              state = (*argument=='1'); ++position;
            } else state = true;

#if cimg_display==0
            print(images,0,"%s mouse cursor for display window%s (skipped, no display support).",
                  state?"Show":"Hide",
                  gmic_selection.data());
#else // #if cimg_display==0
            try {
              if (state) cimg_forY(selection,l) {
                  if (!_display_window[l].is_closed()) _display_window[selection[l]].show_mouse();
                }
              else cimg_forY(selection,l) {
                  if (!_display_window[l].is_closed()) _display_window[selection[l]].hide_mouse();
                }
              print(images,0,"%s mouse cursor for display window%s.",
                    state?"Show":"Hide",
                    gmic_selection.data());
            } catch (CImgDisplayException&) {
              print(images,0,"%s mouse cursor for display window%s (skipped, no display available).",
                    state?"Show":"Hide",
                    gmic_selection.data());
            }
#endif // #if cimg_display==0
            continue;
          }

        } // command1=='c'.

        //----------------------------
        // Commands starting by '-d..'
        //----------------------------
        else if (command1=='d') {

          // Done.
          if (!std::strcmp("-done",item)) {
            const CImg<char> &s = callstack.back();
            if (s[0]!='*' || s[1]!='r')
              error(images,0,0,
                    "Command '-done': Not associated to a '-repeat' command "
                    "within the same scope.");
            *title = 0;
            CImg<unsigned int> &rd = repeatdones.back();
            const unsigned int counter = ++rd[2];
            unsigned int hashcode = ~0U, pos = ~0U;
            if (rd.height()>3) { hashcode = (unsigned int)rd[3]; pos = (unsigned int)rd[4]; }
            if ((rd[2]==~0U && --rd[2]) || --rd[1]) {
              position = rd[0];
              if (hashcode!=~0U) {
                cimg_snprintf(argx,_argx.width(),"%u",counter);
                CImg<char>::string(argx).move_to((*variables[hashcode])[pos]);
              }
              next_debug_line = debug_line; next_debug_filename = debug_filename;
            } else {
              if (is_very_verbose) print(images,0,"End 'repeat..done' block.");
              if (hashcode!=~0U) {
                variables[hashcode]->remove(pos);
                variables_names[hashcode]->remove(pos);
              }
              repeatdones.remove();
              callstack.remove();
            }
            continue;
          }

          // Do..while.
          if (!std::strcmp("-do",item)) {
            if (is_debug_info && debug_line!=~0U) {
              cimg_snprintf(argx,_argx.width(),"*do#%u",debug_line);
              CImg<char>::string(argx).move_to(callstack);
            } else CImg<char>::string("*do").move_to(callstack);
            if (is_very_verbose) print(images,0,"Start '-do..-while' block.");
            CImg<unsigned int>::vector(position).move_to(dowhiles);
            continue;
          }

          // Discard value.
          if (!std::strcmp("-discard",command)) {
            gmic_substitute_args();
            CImg<T> values;
            *argx = 0;

            if (cimg_sscanf(argument,"%255[xyzc]%c",argx,&end)==1) {

              // Discard neighboring duplicate values along axes.
              print(images,0,"Discard neighboring duplicate values along '%s'-ax%cs, in image%s.",
                    argx,
                    std::strlen(argx)>1?'e':'i',
                    gmic_selection.data());
              cimg_forY(selection,l) gmic_apply(gmic_discard(argx));
              ++position;

            } else if (cimg_sscanf(argument,"%255[xyzc]%c",argx,&end)==2 && end==',') {

              // Discard sequence of values along axes.
              unsigned int nb_values = 1, s_argx = (unsigned int)std::strlen(argx) + 1;
              for (const char *s = argument + s_argx; *s; ++s) if (*s==',') ++nb_values;
              try { values.assign(nb_values,1,1,1).fill(argument + s_argx,true,false); }
              catch (CImgException&) { arg_error("discard"); }
              print(images,0,"Discard sequence of values '%s' along '%s'-ax%cs, in image%s.",
                    gmic_argument_text_printed() + s_argx,
                    argx,
                    std::strlen(argx)>1?'e':'i',
                    gmic_selection.data());
              cimg_forY(selection,l) gmic_apply(gmic_discard(values,argx));
              ++position;

            } else { // Discard sequence of values or neighboring duplicate values.
              unsigned int nb_values = *argument?1U:0U;
              for (const char *s = argument; *s; ++s) if (*s==',') ++nb_values;
              try { values.assign(nb_values,1,1,1).fill(argument,true,false); }
              catch (CImgException&) { values.assign(); }
              if (values) {
                print(images,0,"Discard sequence of values '%s' in image%s.",
                      gmic_argument_text_printed(),
                      gmic_selection.data());
                cimg_forY(selection,l) gmic_apply(discard(values));
                ++position;

              } else {
                print(images,0,"Discard neighboring duplicate values in image%s.",
                      gmic_selection.data());
                cimg_forY(selection,l) gmic_apply(discard());
              }
            }
            is_released = false; continue;
          }

          // Enable debug mode (useful when '-debug' is invoked from a custom command).
          if (!std::strcmp("-debug",item)) {
            is_debug = true;
            continue;
          }

          // Divide.
          gmic_arithmetic_command("-div",
                                  operator/=,
                                  "Divide image%s by %g%s",
                                  gmic_selection.data(),value,ssep,Tfloat,
                                  div,
                                  "Divide image%s by image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Divide image%s by expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Divide image%s");

          // Distance function.
          if (!std::strcmp("-distance",command)) {
            gmic_substitute_args();
            unsigned int algorithm = 0, off = 0;
            int metric = 2;
            sep0 = sep1 = 0;
            value = 0;
            if ((cimg_sscanf(argument,"%lf%c",
                             &value,&end)==1 ||
                 (cimg_sscanf(argument,"%lf%c%c",
                              &value,&sep0,&end)==2 && sep0=='%') ||
                 cimg_sscanf(argument,"%lf,%d%c",
                             &value,&metric,&end)==2 ||
                 (cimg_sscanf(argument,"%lf%c,%d%c",
                              &value,&sep0,&metric,&end)==3 && sep0=='%')) &&
                metric>=0 && metric<=3) {
              print(images,0,"Compute distance map to isovalue %g%s in image%s, "
                    "with %s metric.",
                    value,sep0=='%'?"%":"",
                    gmic_selection.data(),
                    metric==0?"chebyshev":metric==1?"manhattan":metric==2?"euclidean":
                    "squared-euclidean");
              cimg_forY(selection,l) {
                CImg<T> &img = gmic_check(images[selection[l]]);
                nvalue = value;
                if (sep0=='%' && img) {
                  vmax = (double)img.max_min(vmin);
                  nvalue = vmin + value*(vmax - vmin)/100;
                }
                gmic_apply(distance((T)nvalue,metric));
              }
            } else if ((((cimg_sscanf(argument,"%lf,[%255[a-zA-Z0-9_.%+-]%c%c",
                                      &value,indices,&sep1,&end)==3 ||
                          (cimg_sscanf(argument,"%lf%c,[%255[a-zA-Z0-9_.%+-]%c%c",
                                       &value,&sep0,indices,&sep1,&end)==4 && sep0=='%')) &&
                         sep1==']') ||
                        ((cimg_sscanf(argument,"%lf,[%255[a-zA-Z0-9_.%+-]],%u%c",
                                     &value,indices,&algorithm,&end)==3 ||
                          (cimg_sscanf(argument,"%lf%c,[%255[a-zA-Z0-9_.%+-]],%u%c",
                                       &value,&sep0,indices,&algorithm,&end)==4 && sep0=='%')) &&
                         algorithm<=4)) &&
                       (ind=selection2cimg(indices,images.size(),images_names,"-distance",true,
                                           false,CImg<char>::empty())).height()==1) {
              print(images,0,"Compute distance map%s to isovalue %g%s in image%s, "
                    "using %s algorithm, with metric [%u].",
                    selection.height()>1?(algorithm>=3?"s and return paths":"s"):
                    (algorithm>=3?" and return path":""),
                    value,sep0=='%'?"%":"",
                    gmic_selection.data(),
                    algorithm==0?"fast-marching":algorithm==1||algorithm==3?
                    "low-connectivity dijkstra":"high-connectivity dijkstra",
                    *ind);
              const CImg<T> custom_metric = gmic_image_arg(*ind);
              if (algorithm<3) cimg_forY(selection,l) {
                  CImg<T> &img = gmic_check(images[selection[l]]);
                  nvalue = value;
                  if (sep0=='%' && img) {
                    vmax = (double)img.max_min(vmin);
                    nvalue = vmin + value*(vmax - vmin)/100;
                  }
                  if (!algorithm) { gmic_apply(distance_eikonal((T)nvalue,custom_metric)); }
                  else gmic_apply(distance_dijkstra((T)nvalue,custom_metric,algorithm==2));
                }
              else cimg_forY(selection,l) {
                  const unsigned int uind = selection[l] + off;
                  CImg<T>& img = gmic_check(images[uind]);
                  nvalue = value;
                  if (sep0=='%' && img) {
                    vmax = (double)img.max_min(vmin);
                    nvalue = vmin + value*(vmax - vmin)/100;
                  }
                  name = images_names[uind];
                  CImg<T> path(1),
                    dist = img.get_distance_dijkstra((T)nvalue,custom_metric,algorithm==4,path);
                  if (is_double_hyphen) {
                    images_names.insert(2,name.copymark());
                    dist.move_to(images,~0U);
                    path.move_to(images,~0U);
                  } else {
                    off+=1;
                    dist.move_to(images[uind].assign());
                    path.move_to(images,uind + 1);
                    images_names[uind] = name;
                    images_names.insert(name.copymark(),uind + 1);
                  }
                }
            } else arg_error("distance");
            is_released = false; ++position; continue;
          }

          // Dilate.
          if (!std::strcmp("-dilate",command)) {
            gmic_substitute_args();
            float sx = 3, sy = 3, sz = 1;
            unsigned int is_normalized = 0;
            boundary = 1;
            sep = 0;
            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                              indices,&sep,&end)==2 && sep==']') ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u%c",
                             indices,&boundary,&end)==2 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u,%u%c",
                             indices,&boundary,&is_normalized,&end)==3) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-dilate",true,
                                    false,CImg<char>::empty())).height()==1 &&
                boundary<=1) {
              print(images,0,"Dilate image%s with mask [%u] and %s boundary conditions, "
                    "with%s normalization.",
                    gmic_selection.data(),
                    *ind,
                    boundary?"neumann":"dirichlet",
                    is_normalized?"":"out");
              const CImg<T> mask = gmic_image_arg(*ind);
              cimg_forY(selection,l) gmic_apply(dilate(mask,boundary,(bool)is_normalized));
            } else if ((cimg_sscanf(argument,"%f%c",
                                    &sx,&end)==1) &&
                       sx>=0) {
              sx = cimg::round(sx);
              print(images,0,"Dilate image%s with mask of size %g and neumann boundary conditions.",
                    gmic_selection.data(),
                    sx);
              cimg_forY(selection,l) gmic_apply(dilate((unsigned int)sx));
            } else if ((cimg_sscanf(argument,"%f,%f%c",
                                    &sx,&sy,&end)==2 ||
                        cimg_sscanf(argument,"%f,%f,%f%c",
                                    &sx,&sy,&sz,&end)==3) &&
                       sx>=0 && sy>=0 && sz>=0) {
              sx = cimg::round(sx);
              sy = cimg::round(sy);
              sz = cimg::round(sz);
              print(images,0,"Dilate image%s with %gx%gx%g mask and neumann boundary conditions.",
                    gmic_selection.data(),
                    sx,sy,sz);
              cimg_forY(selection,l) gmic_apply(dilate((unsigned int)sx,(unsigned int)sy,(unsigned int)sz));
            } else arg_error("dilate");
            is_released = false; ++position; continue;
          }

          // Set double-sided mode for 3d rendering.
          if (!std::strcmp("-double3d",item)) {
            gmic_substitute_args();
            bool state = true;
            if (!argument[1] && (*argument=='0' || *argument=='1')) {
              state = (*argument=='1');
              ++position;
            } else state = true;
            is_double3d = state;
            print(images,0,"%s double-sided mode for 3d rendering.",
                  is_double3d?"Enable":"Disable");
            continue;
          }

          // Patch-based smoothing.
          if (!std::strcmp("-denoise",command)) {
            gmic_substitute_args();
            float sigma_s = 10, sigma_r = 10, smoothness = 1;
            unsigned int is_fast_approximation = 0;
            float psize = 5, rsize = 6;
            if ((cimg_sscanf(argument,"%f%c",
                             &sigma_s,&end)==1 ||
                 cimg_sscanf(argument,"%f,%f%c",
                             &sigma_s,&sigma_r,&end)==2 ||
                 cimg_sscanf(argument,"%f,%f,%f%c",
                             &sigma_s,&sigma_r,&psize,&end)==3 ||
                 cimg_sscanf(argument,"%f,%f,%f,%f%c",
                             &sigma_s,&sigma_r,&psize,&rsize,&end)==4 ||
                 cimg_sscanf(argument,"%f,%f,%f,%f,%f%c",
                             &sigma_s,&sigma_r,&psize,&rsize,&smoothness,&end)==5 ||
                 cimg_sscanf(argument,"%f,%f,%f,%f,%f,%u%c",
                             &sigma_s,&sigma_r,&psize,&rsize,&smoothness,
                             &is_fast_approximation,&end)==6) &&
                sigma_s>=0 && sigma_r>=0 && psize>=0 && rsize>=0 && is_fast_approximation<=1) {
              psize = cimg::round(psize);
              rsize = cimg::round(rsize);
              print(images,0,"Denoise image%s using %gx%g patchs, with standard deviations %lg,%g, "
                    "lookup size %g and smoothness %g.",
                    gmic_selection.data(),
                    psize,
                    psize,
                    sigma_s,
                    sigma_r,
                    rsize,
                    smoothness);
              cimg_forY(selection,l)
                gmic_apply(blur_patch(sigma_s,sigma_r,(unsigned int)psize,(unsigned int)rsize,smoothness,
                                      (bool)is_fast_approximation));
            } else arg_error("denoise");
            is_released = false; ++position; continue;
          }

          // Deriche filter.
          if (!std::strcmp("-deriche",command)) {
            gmic_substitute_args();
            unsigned int order = 0;
            float sigma = 0;
            axis = sep = 0;
            boundary = 1;
            if ((cimg_sscanf(argument,"%f,%u,%c%c",&sigma,&order,&axis,&end)==3 ||
                 (cimg_sscanf(argument,"%f%c,%u,%c%c",&sigma,&sep,&order,&axis,&end)==4 &&
                  sep=='%') ||
                 cimg_sscanf(argument,"%f,%u,%c,%u%c",&sigma,&order,&axis,&boundary,&end)==4 ||
                 (cimg_sscanf(argument,"%f%c,%u,%c,%u%c",
                              &sigma,&sep,&order,&axis,&boundary,&end)==5 && sep=='%')) &&
                sigma>=0 && order<=2 && (axis=='x' || axis=='y' || axis=='z' || axis=='c') &&
                boundary<=1) {
              print(images,0,"Apply %u-order Deriche filter on image%s, along axis '%c' with standard "
                    "deviation %g%s and %s boundary conditions.",
                    order,gmic_selection.data(),axis,
                    sigma,sep=='%'?"%":"",
                    boundary?"neumann":"dirichlet");
              if (sep=='%') sigma = -sigma;
              cimg_forY(selection,l) gmic_apply(deriche(sigma,order,axis,(bool)boundary));
            } else arg_error("deriche");
            is_released = false; ++position; continue;
          }

          // Dijkstra algorithm.
          if (!std::strcmp("-dijkstra",command)) {
            gmic_substitute_args();
            float snode = 0, enode = 0;
            if (cimg_sscanf(argument,"%f,%f%c",&snode,&enode,&end)==2 &&
                snode>=0 && enode>=0) {
              snode = cimg::round(snode);
              enode = cimg::round(enode);
              print(images,0,"Compute minimal path from adjacency matri%s%s with the "
                    "Dijkstra algorithm.",
                    selection.height()>1?"ce":"x",gmic_selection.data());
              unsigned int off = 0;
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l] + off;
                name = images_names[uind];
                if (is_double_hyphen) {
                  CImg<T> path, dist = gmic_check(images[uind]).get_dijkstra((unsigned int)snode,
                                                                             (unsigned int)enode,
                                                                             path);
                  images_names.insert(name.copymark());
                  name.move_to(images_names);
                  dist.move_to(images);
                  path.move_to(images);
                } else {
                  CImg<T> path;
                  gmic_check(images[uind]).dijkstra((unsigned int)snode,(unsigned int)enode,path);
                  images_names.insert(name.get_copymark(),uind + 1);
                  name.move_to(images_names[uind]);
                  images.insert(path,uind + 1);
                  ++off;
                }
              }
            } else arg_error("dijkstra");
            is_released = false; ++position; continue;
          }

          // Estimate displacement field.
          if (!std::strcmp("-displacement",command)) {
            gmic_substitute_args();
            float nb_scales = 0, nb_iterations = 10000, smoothness = 0.1f, precision = 5.0f;
            unsigned int is_backward = 1;
            sep = *argx = 0;
            ind0.assign();
            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                              indices,&sep,&end)==2 && sep==']') ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f%c",
                             indices,&smoothness,&end)==2 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f%c",
                             indices,&smoothness,&precision,&end)==3 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f%c",
                             indices,&smoothness,&precision,&nb_scales,&end)==4 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f%c",
                             indices,&smoothness,&precision,&nb_scales,&nb_iterations,&end)==5 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f,%u%c",
                             indices,&smoothness,&precision,&nb_scales,&nb_iterations,
                             &is_backward,&end)==6 ||
                 (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f,%u,[%255[a-zA-Z0-9_.%+-]%c%c",
                              indices,&smoothness,&precision,&nb_scales,&nb_iterations,
                              &is_backward,argx,&sep,&end)==8 && sep==']')) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-displacement",true,
                                    false,CImg<char>::empty())).height()==1 &&
                precision>=0 && nb_scales>=0 && nb_iterations>=0 && is_backward<=1 &&
                (!*argx || (ind0=selection2cimg(argx,images.size(),images_names,"-displacement",true,
                                                false,CImg<char>::empty())).height()==1)) {
              nb_scales = cimg::round(nb_scales);
              nb_iterations = cimg::round(nb_iterations);
              if (nb_scales) cimg_snprintf(argx,_argx.width(),"%g ",nb_scales); else std::strcpy(argx,"auto-");
              if (ind0) cimg_snprintf(argy,_argy.width()," with guide [%u]",*ind0); else *_argy = 0;

              print(images,0,"Estimate displacement field from source [%u] to image%s, with "
                    "%s smoothness %g, precision %g, %sscales, %g iteration%s, in %s direction%s.",
                    *ind,
                    gmic_selection.data(),
                    smoothness>=0?"isotropic":"anisotropic",cimg::abs(smoothness),
                    precision,
                    argx,
                    nb_iterations,nb_iterations!=1?"s":"",
                    is_backward?"backward":"forward",
                    argy);
              const CImg<T> source = gmic_image_arg(*ind);
              const CImg<T> constraints = ind0?gmic_image_arg(*ind0):CImg<T>::empty();
              cimg_forY(selection,l) gmic_apply(displacement(source,smoothness,precision,(unsigned int)nb_scales,
                                                             (unsigned int)nb_iterations,(bool)is_backward,
                                                             constraints));
            } else arg_error("displacement");
            is_released = false; ++position; continue;
          }

          // Display.
          if (!is_double_hyphen && !std::strcmp("-display",command)) {
            gmic_substitute_args();
            unsigned int XYZ[3];
            bool is_args = false;
            exit_on_anykey = 0;
            if ((cimg_sscanf(argument,"%lf,%lf,%lf%c",&value,&value0,&value1,&end)==3 ||
                 cimg_sscanf(argument,"%lf,%lf,%lf,%u%c",&value,&value0,&value1,&exit_on_anykey,&end)==4) &&
                value>=0 && value0>=0 && value1>=0 && exit_on_anykey<=1) {
              is_args = true;
              XYZ[0] = (unsigned int)cimg::round(value);
              XYZ[1] = (unsigned int)cimg::round(value0);
              XYZ[2] = (unsigned int)cimg::round(value1);
              ++position;
            }
            display_images(images,images_names,selection,is_args?XYZ:0,exit_on_anykey);
            is_released = true; continue;
          }

          // Display 3d object.
          if (!is_double_hyphen && !std::strcmp("-display3d",command)) {
            gmic_substitute_args();
            exit_on_anykey = 0;
            sep = 0;
            if ((cimg_sscanf(argument,"%u%c",
                             &exit_on_anykey,&end)==1 ||
                 (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                              indices,&sep,&end)==2 && sep==']') ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u%c",
                             indices,&exit_on_anykey,&end)==2) &&
                exit_on_anykey<=1 &&
                (*argument!='[' || (ind=selection2cimg(indices,images.size(),images_names,"-display3d",true,
                                                       false,CImg<char>::empty())).height()==1)) ++position;
            if (ind.height()==1) g_img_uc = gmic_image_arg(*ind);
            display_objects3d(images,images_names,selection,g_img_uc,exit_on_anykey);
            g_img_uc.assign();
            is_released = true; continue;
          }

        } // command1=='d'.

        //----------------------------
        // Commands starting by '-e..'
        //----------------------------
        else if (command1=='e') {

          // Endif.
          if (!std::strcmp("-endif",item)) {
            const CImg<char> &s = callstack.back();
            if (s[0]!='*' || s[1]!='i')
              error(images,0,0,
                    "Command '-endif': Not associated to a '-if' command within the same scope.");
            if (is_very_verbose) print(images,0,"End 'if..endif' block.");
            check_elif = false;
            callstack.remove();
            continue;
          }

          // Else and elif.
          if (!std::strcmp("-else",item) || (!std::strcmp("-elif",item) && !check_elif)) {
            const CImg<char> &s = callstack.back();
            if (s[0]!='*' || s[1]!='i')
              error(images,0,0,
                    "Command '%s': Not associated to a '-if' command within the same scope.",
                    item);
            check_elif = false;
            if (is_very_verbose) print(images,0,"Reach '-else' block.");
            for (int nb_ifs = 1; nb_ifs && position<commands_line.size(); ++position) {
              const char *const it = commands_line[position].data();
              if (*it==1 &&
                  cimg_sscanf(commands_line[position].data() + 1,"%x,%x",&_debug_line,&(_debug_filename=0))>0) {
                is_debug_info = true; next_debug_line = _debug_line; next_debug_filename = _debug_filename;
              } else if (!std::strcmp("-if",it)) ++nb_ifs;
              else if (!std::strcmp("-endif",it)) { if (!--nb_ifs) --position; }
            }
            continue;
          }

          // End local environment.
          if (!std::strcmp("-endlocal",item) || !std::strcmp("-endl",item)) {
            const CImg<char> &s = callstack.back();
            if (s[0]!='*' || s[1]!='l')
              error(images,0,0,
                    "Command '-endlocal': Not associated to a '-local' command within "
                    "the same scope.");
            if (is_very_verbose) print(images,0,"End 'local..endlocal' block.");
            is_endlocal = true;
            break;
          }

          // Echo.
          if (!is_double_hyphen && is_echo_command) {
            if (is_verbose) {
              gmic_substitute_args();
              name.assign(argument,(unsigned int)std::strlen(argument) + 1);
              cimg::strunescape(name);
              if (is_restriction) print(images,&selection,"%s",name.data());
              else print(images,0,"%s",name.data());
            }
            ++position; continue;
          }

          // Exec.
          if (!std::strcmp("-exec",item)) {
            gmic_substitute_args();
#ifdef gmic_noexec
            print(images,0,"Execute external command '%s' (skipped, no exec allowed).",
                  gmic_argument_text_printed());
#else // #ifdef gmic_noexec
            print(images,0,"Execute external command '%s'\n",
                  gmic_argument_text_printed());
            name.assign(argument,(unsigned int)std::strlen(argument) + 1);
            cimg::strunescape(name);
            strreplace_fw(name);
            cimg::mutex(31);
            const int errcode = cimg::system(name);
            cimg::mutex(31,0);
            cimg_snprintf(title,_title.width(),"%d",errcode);
            CImg<char>::string(title).move_to(status);
            if (errcode) print(images,0,"Command '-exec' returned error code '%d'.",
                               errcode);
#endif // #ifdef gmic_noexec
            ++position; continue;
          }

          // Error.
          if (!is_double_hyphen && !std::strcmp("-error",command)) {
            gmic_substitute_args();
            name.assign(argument,(unsigned int)std::strlen(argument) + 1);
            cimg::strunescape(name);
            if (is_restriction) error(images,&selection,0,"%s",name.data());
            else error(images,0,0,"%s",name.data());
          }

          // Invert endianness.
          if (!std::strcmp("-endian",item)) {
            gmic_substitute_args();
            if (!std::strcmp(argument,"uchar") ||
                !std::strcmp(argument,"unsigned char") || !std::strcmp(argument,"char") ||
                !std::strcmp(argument,"ushort") || !std::strcmp(argument,"unsigned short") ||
                !std::strcmp(argument,"short") || !std::strcmp(argument,"uint") ||
                !std::strcmp(argument,"unsigned int") || !std::strcmp(argument,"int") ||
                !std::strcmp(argument,"ulong") || !std::strcmp(argument,"unsigned long") ||
                !std::strcmp(argument,"long") || !std::strcmp(argument,"float") ||
                !std::strcmp(argument,"double")) {
              print(images,0,"Invert data endianness of image%s, with assumed pixel type '%s'.",
                    gmic_selection.data(),argument);
              ++position;
            } else print(images,0,"Invert data endianness of image%s.",
                         gmic_selection.data());
            cimg_forY(selection,l) gmic_apply(gmic_invert_endianness(argument));
            is_released = false; continue;
          }

          // Exponential.
          gmic_simple_command("-exp",exp,"Compute pointwise exponential of image%s.");

          // Test equality.
          gmic_arithmetic_command("-eq",
                                  operator_eq,
                                  "Compute boolean equality between image%s and %g%s",
                                  gmic_selection.data(),value,ssep,T,
                                  operator_eq,
                                  "Compute boolean equality between image%s and image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute boolean equality between image%s and expression %s'",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute boolean equality between image%s");

          // Draw ellipse.
          if (!std::strcmp("-ellipse",command)) {
            gmic_substitute_args();
            float x = 0, y = 0, R = 0, r = 0, angle = 0;
            sep0 = sep1 = sepx = sepy = *argx = *argy = *argz = *argc = *color = 0;
            pattern = ~0U; opacity = 1;
            if ((cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             argx,argy,argz,&end)==3 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-]%c",
                             argx,argy,argz,argc,&end)==4 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-],%f%c",
                             argx,argy,argz,argc,&angle,&end)==5 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-],%f,%f%c",
                             argx,argy,argz,argc,&angle,&opacity,&end)==6 ||
                 (cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                              "%255[0-9.eE%+-],%f,%f,0%c%x%c",
                              argx,argy,argz,argc,&angle,&opacity,&sep1,&pattern,
                              &end)==8 &&
                  sep1=='x') ||
                 (cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                              "%255[0-9.eE%+-],%f,%f,%4095[0-9.eEinfa,+-]%c",
                              argx,argy,argz,argc,&angle,&opacity,color,&end)==7 &&
                  (bool)(pattern=~0U))||
                 (*color=0,cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                                       "%255[0-9.eE%+-],%f,%f,0%c%x,%4095[0-9.eEinfa,+-]%c",
                                       argx,argy,argz,argc,&angle,&opacity,&sep1,
                                       &pattern,color,&end)==9 &&
                  sep1=='x')) &&
                (cimg_sscanf(argx,"%f%c",&x,&end)==1 ||
                 (cimg_sscanf(argx,"%f%c%c",&x,&sepx,&end)==2 && sepx=='%')) &&
                (cimg_sscanf(argy,"%f%c",&y,&end)==1 ||
                 (cimg_sscanf(argy,"%f%c%c",&y,&sepy,&end)==2 && sepy=='%')) &&
                (cimg_sscanf(argz,"%f%c",&R,&end)==1 ||
                 (cimg_sscanf(argz,"%f%c%c",&R,&sep1,&end)==2 && sep1=='%')) &&
                (!*argc ||
                 cimg_sscanf(argc,"%f%c",&r,&end)==1 ||
                 (cimg_sscanf(argc,"%f%c%c",&r,&sep0,&end)==2 && sep0=='%'))) {
              if (!*argc) r = R;
              print(images,0,"Draw %s ellipse at (%g%s,%g%s) with radii (%g%s,%g%s) on image%s, "
                    "with orientation %g°, opacity %g and color (%s).",
                    sep1=='x'?"outlined":"filled",
                    x,sepx=='%'?"%":"",
                    y,sepy=='%'?"%":"",
                    R,sep1=='%'?"%":"",
                    r,sep0=='%'?"%":"",
                    gmic_selection.data(),
                    angle,
                    opacity,
                    *color?color:"default");
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                g_img.assign(img.spectrum(),1,1,1,0).fill(color,true,false);
                const float rmax = std::sqrt((float)cimg::sqr(img.width()) +
                                             cimg::sqr(img.height()));
                const int
                  nx = (int)cimg::round(sepx=='%'?x*(img.width() - 1)/100:x),
                  ny = (int)cimg::round(sepy=='%'?y*(img.height() - 1)/100:y);
                const float
                  nR = cimg::round(sep1=='%'?R*rmax/100:R),
                  nr = cimg::round(sep0=='%'?r*rmax/100:r);
                if (sep1=='x') {
                  if (nR==nr) { gmic_apply(draw_circle(nx,ny,(int)nR,g_img.data(),opacity,~0U)); }
                  else gmic_apply(draw_ellipse(nx,ny,nR,nr,angle,g_img.data(),opacity,~0U));
                } else {
                  if (nR==nr) { gmic_apply(draw_circle(nx,ny,(int)nR,g_img.data(),opacity)); }
                  else gmic_apply(draw_ellipse(nx,ny,nR,nr,angle,g_img.data(),opacity));
                }
              }
            } else arg_error("ellipse");
            g_img.assign();
            is_released = false; ++position; continue;
          }

          // Equalize.
          if (!std::strcmp("-equalize",command)) {
            gmic_substitute_args();
            float nb_levels = 256;
            bool no_min_max = false;
            sep = sep0 = sep1 = 0;
            value0 = value1 = 0;
            if (((cimg_sscanf(argument,"%f%c",
                              &nb_levels,&end)==1 && (no_min_max=true)) ||
                 ((cimg_sscanf(argument,"%f%c%c",
                               &nb_levels,&sep,&end)==2 && sep=='%') && (no_min_max=true)) ||
                 cimg_sscanf(argument,"%f,%lf,%lf%c",
                             &nb_levels,&value0,&value1,&end)==3 ||
                 (cimg_sscanf(argument,"%f%c,%lf,%lf%c",
                              &nb_levels,&sep,&value0,&value1,&end)==4 && sep=='%') ||
                 (cimg_sscanf(argument,"%f,%lf%c,%lf%c",
                              &nb_levels,&value0,&sep0,&value1,&end)==4 && sep0=='%') ||
                 (cimg_sscanf(argument,"%f%c,%lf%c,%lf%c",
                              &nb_levels,&sep,&value0,&sep0,&value1,&end)==5 && sep=='%' &&
                  sep0=='%') ||
                 (cimg_sscanf(argument,"%f,%lf,%lf%c%c",
                              &nb_levels,&value0,&value1,&sep1,&end)==4 && sep1=='%') ||
                 (cimg_sscanf(argument,"%f%c,%lf,%lf%c%c",
                              &nb_levels,&sep,&value0,&value1,&sep1,&end)==5 && sep=='%' &&
                  sep1=='%') ||
                 (cimg_sscanf(argument,"%f,%lf%c,%lf%c%c",
                              &nb_levels,&value0,&sep0,&value1,&sep1,&end)==5 && sep0=='%' &&
                  sep1=='%') ||
                 (cimg_sscanf(argument,"%f%c,%lf%c,%lf%c%c",
                              &nb_levels,&sep,&value0,&sep0,&value1,&sep1,&end)==6 && sep=='%' &&
                  sep0=='%' && sep1=='%')) &&
                nb_levels>=0.5) { nb_levels = cimg::round(nb_levels); ++position; }
            else { nb_levels = 256; value0 = 0; value1 = 100; sep = 0; sep0 = sep1 = '%'; }
            if (no_min_max) { value0 = 0; value1 = 100; sep0 = sep1 = '%'; }
            print(images,0,"Equalize histogram of image%s, with %g%s levels in range [%g%s,%g%s].",
                  gmic_selection.data(),
                  nb_levels,sep=='%'?"%":"",
                  value0,sep0=='%'?"%":"",
                  value1,sep1=='%'?"%":"");
            cimg_forY(selection,l) {
              CImg<T>& img = gmic_check(images[selection[l]]);
              nvalue0 = value0; nvalue1 = value1;
              vmin = vmax = 0;
              if (sep0=='%' || sep1=='%') {
                if (img) vmax = (double)img.max_min(vmin);
                if (sep0=='%') nvalue0 = vmin + (vmax - vmin)*value0/100;
                if (sep1=='%') nvalue1 = vmin + (vmax - vmin)*value1/100;
              }
              const unsigned int
                _nb_levels = cimg::max(1U,
                                       (unsigned int)cimg::round(sep=='%'?
                                                                 nb_levels*(1 + nvalue1 - nvalue0)/100:
                                                                 nb_levels));
              gmic_apply(equalize(_nb_levels,(T)nvalue0,(T)nvalue1));
            }
            is_released = false; continue;
          }

          // Erode.
          if (!std::strcmp("-erode",command)) {
            gmic_substitute_args();
            unsigned int is_normalized = 0;
            float sx = 3, sy = 3, sz = 1;
            boundary = 1;
            sep = 0;
            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                              indices,&sep,&end)==2 && sep==']') ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u%c",
                             indices,&boundary,&end)==2 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u,%u%c",
                             indices,&boundary,&is_normalized,&end)==3) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-erode",true,
                                    false,CImg<char>::empty())).height()==1 &&
                boundary<=1) {
              print(images,0,"Erode image%s with mask [%u] and %s boundary conditions, "
                    "with%s normalization.",
                    gmic_selection.data(),
                    *ind,
                    boundary?"neumann":"dirichlet",
                    is_normalized?"":"out");
              const CImg<T> mask = gmic_image_arg(*ind);
              cimg_forY(selection,l) gmic_apply(erode(mask,boundary,(bool)is_normalized));
            } else if ((cimg_sscanf(argument,"%f%c",
                                    &sx,&end)==1) &&
                       sx>=0) {
              sx = cimg::round(sx);
              print(images,0,"Erode image%s with mask of size %g and neumann boundary conditions.",
                    gmic_selection.data(),
                    sx);
              cimg_forY(selection,l) gmic_apply(erode((unsigned int)sx));
            } else if ((cimg_sscanf(argument,"%f,%f%c",
                                    &sx,&sy,&end)==2 ||
                        cimg_sscanf(argument,"%f,%f,%f%c",
                                    &sx,&sy,&sz,&end)==3) &&
                       sx>=0 && sy>=0 && sz>=0) {
              sx = cimg::round(sx);
              sy = cimg::round(sy);
              sz = cimg::round(sz);
              print(images,0,"Erode image%s with %gx%gx%g mask and neumann boundary conditions.",
                    gmic_selection.data(),
                    sx,sy,sz);
              cimg_forY(selection,l) gmic_apply(erode((unsigned int)sx,(unsigned int)sy,(unsigned int)sz));
            } else arg_error("erode");
            is_released = false; ++position; continue;
          }

          // Build 3d elevation.
          if (!std::strcmp("-elevation3d",command)) {
            gmic_substitute_args();
            float fact = 1;
            sep = *formula = 0;
            if (cimg_sscanf(argument,"'%4095[^']'%c",formula,&end)==1) {
              print(images,0,"Build 3d elevation of image%s, with elevation formula '%s'.",
                    gmic_selection.data(),
                    formula);
              cimg_forY(selection,l) {
                CImg<T>& img = gmic_check(images[selection[l]]);
                CImg<typename CImg<T>::Tfloat> elev(img.width(),img.height(),1,1,formula,true);
                img.get_elevation3d(primitives,g_list_f,elev).move_to(vertices);
                vertices.object3dtoCImg3d(primitives,g_list_f,false);
                gmic_apply(replace(vertices));
                primitives.assign();
              }
              ++position;
            } else if (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep,&end)==2 &&
                       sep==']' &&
                       (ind=selection2cimg(indices,images.size(),images_names,"-elevation3d",true,
                                           false,CImg<char>::empty())).height()==1) {
              print(images,0,"Build 3d elevation of image%s, with elevation map [%u].",
                    gmic_selection.data(),
                    *ind);
              CImg<typename CImg<T>::Tfloat> elev;
              if (images[*ind].spectrum()>1) images[*ind].get_norm().move_to(elev);
              else elev = gmic_image_arg(*ind);
              cimg_forY(selection,l) {
                CImg<T>& img = gmic_check(images[selection[l]]);
                img.get_elevation3d(primitives,g_list_f,elev).move_to(vertices);
                vertices.object3dtoCImg3d(primitives,g_list_f,false);
                gmic_apply(replace(vertices));
                primitives.assign();
              }
              ++position;
            } else {
              if (cimg_sscanf(argument,"%f%c",
                              &fact,&end)==1) {
                print(images,0,"Build 3d elevation of image%s, with elevation factor %g.",
                      gmic_selection.data(),
                      fact);
                ++position;
              } else
                print(images,0,"Build 3d elevation of image%s.",
                      gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T>& img = gmic_check(images[selection[l]]);
                CImg<typename CImg<T>::Tfloat> elev;
                if (fact==1 && img.spectrum()==1) elev = img.get_shared();
                else if (img.spectrum()>1) (img.get_norm().move_to(elev))*=fact;
                else (elev = img)*=fact;
                img.get_elevation3d(primitives,g_list_f,elev).move_to(vertices);
                vertices.object3dtoCImg3d(primitives,g_list_f,false);
                gmic_apply(replace(vertices));
                primitives.assign();
              }
            }
            g_list_f.assign();
            is_released = false; continue;
          }

          // Eigenvalues/eigenvectors.
          if (!std::strcmp("-eigen",command)) {
            print(images,0,"Compute eigen-values/vectors of symmetric matri%s or matrix field%s.",
                  selection.height()>1?"ce":"x",gmic_selection.data());
            unsigned int off = 0;
            cimg_forY(selection,l) {
              const unsigned int uind = selection[l] + off;
              name = images_names[uind];
              CImg<float> val, vec;
              gmic_check(images[uind]).gmic_symmetric_eigen(val,vec);
              if (is_double_hyphen) {
                images_names.insert(name.copymark());
                name.move_to(images_names);
                val.move_to(images);
                vec.move_to(images);
              } else {
                images_names.insert(name.get_copymark(),uind + 1); name.move_to(images_names[uind]);
                val.move_to(images[uind].assign()); images.insert(vec,uind + 1);
                ++off;
              }
            }
            is_released = false; continue;
          }

        } // command1=='e'.

        //----------------------------
        // Commands starting by '-f..'
        //----------------------------
        else if (command1=='f') {

          // Fill.
          if (!std::strcmp("-fill",command)) {
            gmic_substitute_args();
            value = 0;
            sep = 0;
            if (cimg_sscanf(argument,"%lf%c",
                            &value,&end)==1) {
              print(images,0,"Fill image%s with %g.",
                    gmic_selection.data(),
                    value);
              cimg_forY(selection,l) gmic_apply(fill((T)value));
            } else if (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep,&end)==2 &&
                       sep==']' &&
                       (ind=selection2cimg(indices,images.size(),images_names,"-fill",true,
                                           false,CImg<char>::empty())).height()==1) {
              print(images,0,"Fill image%s with values from image [%u].",
                    gmic_selection.data(),
                    *ind);
              const CImg<T> values = gmic_image_arg(*ind);
              cimg_forY(selection,l) gmic_apply(fill(values));
            } else {
              gmic_argument_text_printed();
              if (*argument_text=='\'') cimg::strpare(argument_text,'\'',true,false);
              print(images,0,"Fill image%s with expression '%s'.",
                    gmic_selection.data(),
                    argument_text.data());
              name.assign(argument,(unsigned int)std::strlen(argument) + 1);
              cimg::strpare(name,'\'',true,false);
              strreplace_fw(name);
              cimg_forY(selection,l) gmic_apply(fill(name.data(),true,true,&images,&images));
            }
            is_released = false; ++position; continue;
          }

          // Flood fill.
          if (!std::strcmp("-flood",command)) {
            gmic_substitute_args();
            float x = 0, y = 0, z = 0, tolerance = 0;
            unsigned int is_high_connectivity = 0;
            sepx = sepy = sepz = *argx = *argy = *argz = *color = 0;
            opacity = 1;
            if ((cimg_sscanf(argument,"%255[0-9.eE%+-]%c",
                             argx,&end)==1 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             argx,argy,&end)==2 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             argx,argy,argz,&end)==3 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eEinfa%+-],%f%c",
                             argx,argy,argz,&tolerance,&end)==4 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eEinfa%+-],%f,%u%c",
                             argx,argy,argz,&tolerance,&is_high_connectivity,&end)==5 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eEinfa%+-],%f,%u,%f%c",
                             argx,argy,argz,&tolerance,&is_high_connectivity,&opacity,&end)==6 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eEinfa%+-],%f,%u,%f,"
                             "%4095[0-9.eEinfa,+-]%c",
                             argx,argy,argz,&tolerance,&is_high_connectivity,
                             &opacity,color,&end)==7) &&
                (cimg_sscanf(argx,"%f%c",&x,&end)==1 ||
                 (cimg_sscanf(argx,"%f%c%c",&x,&sepx,&end)==2 && sepx=='%')) &&
                (!*argy ||
                 cimg_sscanf(argy,"%f%c",&y,&end)==1 ||
                 (cimg_sscanf(argy,"%f%c%c",&y,&sepy,&end)==2 && sepy=='%')) &&
                (!*argz ||
                 cimg_sscanf(argz,"%f%c",&z,&end)==1 ||
                 (cimg_sscanf(argz,"%f%c%c",&z,&sepz,&end)==2 && sepz=='%')) &&
                tolerance>=0) {
              print(images,0,
                    "Flood fill image%s from (%g%s,%g%s,%g%s), with tolerance %g, %s connectivity,"
                    "opacity %g and color (%s).",
                    gmic_selection.data(),
                    x,sepx=='%'?"%":"",
                    y,sepy=='%'?"%":"",
                    z,sepz=='%'?"%":"",
                    tolerance,
                    is_high_connectivity?"high":"low",
                    opacity,
                    *color?color:"default");
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                g_img.assign(img.spectrum(),1,1,1,0).fill(color,true,false);
                const int
                  nx = (int)cimg::round(sepx=='%'?x*(img.width() - 1)/100:x),
                  ny = (int)cimg::round(sepy=='%'?y*(img.height() - 1)/100:y),
                  nz = (int)cimg::round(sepz=='%'?z*(img.depth() - 1)/100:z);
                gmic_apply(draw_fill(nx,ny,nz,g_img.data(),opacity,tolerance,(bool)is_high_connectivity));
              }
            } else arg_error("flood");
            g_img.assign();
            is_released = false; ++position; continue;
          }

          // List of directory files.
          if (!std::strcmp("-files",item)) {
            gmic_substitute_args();
            unsigned int mode = 5;
            if ((*argument>='0' && *argument<='5') &&
                argument[1]==',' && argument[2]) {
              mode = (unsigned int)(*argument - '0');
              argument+=2;
            }
            const unsigned int _mode = mode%3;
            print(images,0,"Get list of %s from location '%s'.",
                  _mode==0?"files":_mode==1?"folders":"files and folders",
                  argument);
            g_list_c = cimg::files(argument,true,_mode,mode>=3);
            cimglist_for(g_list_c,l) {
              strreplace_bw(g_list_c[l]);
              g_list_c[l].back() = ',';
            }
            if (g_list_c) {
              g_list_c.back().back() = 0;
              (g_list_c>'x').move_to(status);
            } else status.assign();
            g_list_c.assign();
            ++position; continue;
          }

          // Set 3d focale.
          if (!std::strcmp("-focale3d",item)) {
            gmic_substitute_args();
            value = 700;
            if (cimg_sscanf(argument,"%lf%c",&value,&end)==1) ++position;
            else value = 700;
            focale3d = (float)value;
            print(images,0,"Set 3d focale to %g.",
                  focale3d);
            continue;
          }

        } // command1=='f'.

        //----------------------------
        // Commands starting by '-g..'
        //----------------------------
        else if (command1=='g') {

          // Greater or equal.
          gmic_arithmetic_command("-ge",
                                  operator_ge,
                                  "Compute boolean 'greater or equal than' between image%s and %g%s",
                                  gmic_selection.data(),value,ssep,T,
                                  operator_ge,
                                  "Compute boolean 'greater or equal than' between image%s "
                                  "and image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute boolean 'greater or equal than' between image%s "
                                  "and expression %s'",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute boolean 'greater or equal than' between image%s");

          // Greater than.
          gmic_arithmetic_command("-gt",
                                  operator_gt,
                                  "Compute boolean 'greater than' between image%s and %g%s",
                                  gmic_selection.data(),value,ssep,T,
                                  operator_gt,
                                  "Compute boolean 'greater than' between image%s and image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute boolean 'greater than' between image%s and expression %s'",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute boolean 'greater than' between image%s");

          // Compute gradient.
          if (!std::strcmp("-gradient",command)) {
            gmic_substitute_args();
            int scheme = 3;
            *argx = 0;
            if ((cimg_sscanf(argument,"%15[xyz]%c",
                             argx,&end)==1 ||
                 cimg_sscanf(argument,"%15[xyz],%d%c",
                             argx,&scheme,&end)==2) &&
                scheme>=-1 && scheme<=5) {
              ++position;
              print(images,0,"Compute gradient of image%s along axes '%s', with %s scheme.",
                    gmic_selection.data(),
                    argx,
                    scheme==-1?"backward differences":scheme==4?"deriche":scheme==5?"vanvliet":
                    scheme==1?"forward differences":scheme==2?"sobel":
                    scheme==3?"rotation invariant":"centered differences");
            } else print(images,0,"Compute gradient of image%s, with rotation invariant scheme.",
                         gmic_selection.data());
            unsigned int off = 0;
            cimg_forY(selection,l) {
              const unsigned int uind = selection[l] + off;
              CImg<T>& img = gmic_check(images[uind]);
              g_list = img.get_gradient(*argx?argx:0,scheme);
              name = images_names[uind];
              if (is_double_hyphen) {
                images_names.insert(g_list.size(),name.copymark());
                g_list.move_to(images,~0U);
              } else {
                off+=g_list.size() - 1;
                g_list[0].move_to(images[uind].assign());
                for (unsigned int i = 1; i<g_list.size(); ++i) g_list[i].move_to(images,uind + i);
                images_names[uind] = name;
                if (g_list.size()>1)
                  images_names.insert(g_list.size() - 1,name.copymark(),uind + 1);
              }
            }
            g_list.assign();
            is_released = false; continue;
          }

          // Guided filter.
          if (!std::strcmp("-guided",command)) {
            gmic_substitute_args();
            float radius = 0, regularization = 0;
            *argz = *argc = 0;
            sep =  0;
            if (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                            indices,argx,argy,&end)==3 &&
                (cimg_sscanf(argx,"%f%c",&radius,&end)==1 ||
                 (cimg_sscanf(argx,"%f%c%c",&radius,&sep,&end)==2 && sep=='%')) &&
                cimg_sscanf(argy,"%f%c",&regularization,&end)==1 &&
                (ind=selection2cimg(indices,images.size(),images_names,"-guided",true,
                                    false,CImg<char>::empty())).height()==1 &&
                radius>=0 && regularization>=0) {
              print(images,0,"Apply guided filter on image%s, with guide image [%u], "
                    "radius %g%s and regularization %g.",
                    gmic_selection.data(),
                    *ind,
                    radius,sep=='%'?"%":"",
                    regularization);
              const CImg<T> guide = gmic_image_arg(*ind);
              if (sep=='%') radius = -radius;
              cimg_forY(selection,l) gmic_apply(blur_guided(guide,radius,regularization));
            } else if (cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                                    argx,argy,&end)==2 &&
                       (cimg_sscanf(argx,"%f%c",&radius,&end)==1 ||
                        (cimg_sscanf(argx,"%f%c%c",&radius,&sep,&end)==2 && sep=='%')) &&
                       cimg_sscanf(argy,"%f%c",&regularization,&end)==1 &&
                       radius>=0 && regularization>=0) {
              print(images,0,"Apply guided filter on image%s, with radius %g%s and regularization %g.",
                    gmic_selection.data(),
                    radius,sep=='%'?"%":"",
                    regularization);
              if (sep=='%') radius = -radius;
              cimg_forY(selection,l) gmic_apply(blur_guided(images[selection[l]],radius,regularization));
            } else arg_error("guided");
            is_released = false; ++position; continue;
          }

          // Draw graph.
          if (!std::strcmp("-graph",command)) {
            gmic_substitute_args();
            double ymin = 0, ymax = 0, xmin = 0, xmax = 0;
            unsigned int plot_type = 1, vertex_type = 1;
            float resolution = 65536;
            *formula = *color = sep = sep1 = 0;
            pattern = ~0U; opacity = 1;
            if (((cimg_sscanf(argument,"'%1023[^']%c%c",
                              formula,&sep,&end)==2 && sep=='\'') ||
                 cimg_sscanf(argument,"'%1023[^']',%f%c",
                             formula,&resolution,&end)==2 ||
                 cimg_sscanf(argument,"'%1023[^']',%f,%u%c",
                             formula,&resolution,&plot_type,&end)==3 ||
                 cimg_sscanf(argument,"'%1023[^']',%f,%u,%u%c",
                             formula,&resolution,&plot_type,&vertex_type,&end)==4 ||
                 cimg_sscanf(argument,"'%1023[^']',%f,%u,%u,%lf,%lf%c",
                             formula,&resolution,&plot_type,&vertex_type,&xmin,&xmax,&end)==6 ||
                 cimg_sscanf(argument,"'%1023[^']',%f,%u,%u,%lf,%lf,%lf,%lf%c",
                             formula,&resolution,&plot_type,&vertex_type,&xmin,&xmax,
                             &ymin,&ymax,&end)==8 ||
                 cimg_sscanf(argument,"'%1023[^']',%f,%u,%u,%lf,%lf,%lf,%lf,%f%c",
                             formula,&resolution,&plot_type,&vertex_type,
                             &xmin,&xmax,&ymin,&ymax,&opacity,&end)==9 ||
                 (cimg_sscanf(argument,"'%1023[^']',%f,%u,%u,%lf,%lf,%lf,%lf,%f,0%c%x%c",
                              formula,&resolution,&plot_type,&vertex_type,&xmin,&xmax,
                              &ymin,&ymax,&opacity,&sep1,&pattern,&end)==11 && sep1=='x') ||
                 (cimg_sscanf(argument,"'%1023[^']',%f,%u,%u,%lf,%lf,%lf,%lf,%f,%4095[0-9.eEinfa,+-]%c",
                              formula,&resolution,&plot_type,&vertex_type,&xmin,&xmax,&ymin,&ymax,
                              &opacity,color,&end)==10 && (bool)(pattern=~0U)) ||
                 (*color=0,cimg_sscanf(argument,"'%1023[^']',%f,%u,%u,%lf,%lf,%lf,%lf,%f,0%c%x,"
                                       "%4095[0-9.eEinfa,+-]%c",
                                       formula,&resolution,&plot_type,&vertex_type,&xmin,&xmax,
                                       &ymin,&ymax,&opacity,&sep1,&pattern,color,&end)==12 &&
                  sep1=='x')) &&
                resolution>0 && plot_type<=3 && vertex_type<=7) {
              resolution = cimg::round(resolution);
              strreplace_fw(formula);
              print(images,0,
                    "Draw graph of formula '%s' on image%s, with resolution %g, %s contours, "
                    "%s vertices, x-range = (%g,%g), y-range = (%g,%g), opacity %g, "
                    "pattern 0x%x and color (%s).",
                    formula,
                    gmic_selection.data(),
                    resolution,
                    plot_type==0?"no":plot_type==1?"linear":plot_type==2?"spline":"bar",
                    vertex_type==0?"no":vertex_type==1?"dot":vertex_type==2?"straight cross":
                    vertex_type==3?"diagonal cross":vertex_type==4?"filled circle":
                    vertex_type==5?"outlined circle":vertex_type==6?"square":"diamond",
                    xmin,xmax,
                    ymin,ymax,
                    opacity,pattern,
                    *color?color:"default");
              if (xmin==0 && xmax==0) { xmin = -4; xmax = 4; }
              if (!plot_type && !vertex_type) plot_type = 1;
              if (resolution<1) resolution = 65536;

              CImg<double> values(4,(unsigned int)resolution--,1,1,0);
              const double dx = xmax - xmin;
              cimg_forY(values,X) values(0,X) = xmin + X*dx/resolution;
              cimg::eval(formula,values).move_to(values);

              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                g_img.assign(img.spectrum(),1,1,1,0).fill(color,true,false);
                gmic_apply(draw_graph(values,g_img.data(),opacity,plot_type,vertex_type,ymin,ymax,pattern));
              }
            } else if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                                     indices,&sep,&end)==2 && sep==']') ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u%c",
                                    indices,&plot_type,&end)==2 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u,%u%c",
                                    indices,&plot_type,&vertex_type,&end)==3 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u,%u,%lf,%lf%c",
                                    indices,&plot_type,&vertex_type,&ymin,&ymax,&end)==5 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u,%u,%lf,%lf,%f%c",
                                    indices,&plot_type,&vertex_type,&ymin,&ymax,&opacity,&end)==6||
                        (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u,%u,%lf,%lf,%f,0%c%x%c",
                                     indices,&plot_type,&vertex_type,&ymin,&ymax,&opacity,&sep1,
                                     &pattern,&end)==8 &&
                         sep1=='x') ||
                        (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u,%u,%lf,%lf,%f,"
                                     "%4095[0-9.eEinfa,+-]%c",
                                     indices,&plot_type,&vertex_type,&ymin,&ymax,&opacity,
                                     color,&end)==7 &&
                         (bool)(pattern=~0U)) ||
                        (*color=0,cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u,%u,%lf,%lf,"
                                              "%f,0%c%x,%4095[0-9.eEinfa,+-]%c",
                                              indices,&plot_type,&vertex_type,&ymin,&ymax,
                                              &opacity,&sep1,&pattern,color,&end)==9 &&
                         sep1=='x')) &&
                       (ind=selection2cimg(indices,images.size(),images_names,"-graph",true,
                                           false,CImg<char>::empty())).height()==1 &&
                       plot_type<=3 && vertex_type<=7) {
              if (!plot_type && !vertex_type) plot_type = 1;
              print(images,0,"Draw graph of dataset [%u] on image%s, with %s contours, %s vertices, "
                    "y-range = (%g,%g), opacity %g, pattern 0x%x and color (%s).",
                    *ind,
                    gmic_selection.data(),
                    plot_type==0?"no":plot_type==1?"linear":plot_type==2?"spline":"bar",
                    vertex_type==0?"no":vertex_type==1?"dot":vertex_type==2?"straight cross":
                    vertex_type==3?"diagonal cross":vertex_type==4?"filled circle":
                    vertex_type==5?"outlined circle":vertex_type==6?"square":"diamond",
                    ymin,ymax,
                    opacity,pattern,
                    *color?color:"default");
              const CImg<T> values = gmic_image_arg(*ind);
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                g_img.assign(img.spectrum(),1,1,1,0).fill(color,true,false);
                gmic_apply(draw_graph(values,g_img.data(),opacity,plot_type,vertex_type,ymin,ymax,pattern));
              }
            } else arg_error("graph");
            g_img.assign();
            is_released = false; ++position; continue;
          }

        } // command1=='g'.

        //----------------------------
        // Commands starting by '-h..'
        //----------------------------
        else if (command1=='h') {

          // Histogram.
          if (!std::strcmp("-histogram",command)) {
            gmic_substitute_args();
            float nb_levels = 256;
            bool no_min_max = false;
            sep = sep0 = sep1 = 0;
            value0 = value1 = 0;
            if (((cimg_sscanf(argument,"%f%c",
                              &nb_levels,&end)==1 && (no_min_max=true)) ||
                 ((cimg_sscanf(argument,"%f%c%c",
                               &nb_levels,&sep,&end)==2 && sep=='%') && (no_min_max=true)) ||
                 cimg_sscanf(argument,"%f,%lf,%lf%c",
                             &nb_levels,&value0,&value1,&end)==3 ||
                 (cimg_sscanf(argument,"%f%c,%lf,%lf%c",
                              &nb_levels,&sep,&value0,&value1,&end)==4 && sep=='%') ||
                 (cimg_sscanf(argument,"%f,%lf%c,%lf%c",
                              &nb_levels,&value0,&sep0,&value1,&end)==4 && sep0=='%') ||
                 (cimg_sscanf(argument,"%f%c,%lf%c,%lf%c",
                              &nb_levels,&sep,&value0,&sep0,&value1,&end)==5 && sep=='%' &&
                  sep0=='%') ||
                 (cimg_sscanf(argument,"%f,%lf,%lf%c%c",
                              &nb_levels,&value0,&value1,&sep1,&end)==4 && sep1=='%') ||
                 (cimg_sscanf(argument,"%f%c,%lf,%lf%c%c",
                              &nb_levels,&sep,&value0,&value1,&sep1,&end)==5 && sep=='%' &&
                  sep1=='%') ||
                 (cimg_sscanf(argument,"%f,%lf%c,%lf%c%c",
                              &nb_levels,&value0,&sep0,&value1,&sep1,&end)==5 && sep0=='%' &&
                  sep1=='%') ||
                 (cimg_sscanf(argument,"%f%c,%lf%c,%lf%c%c",
                              &nb_levels,&sep,&value0,&sep0,&value1,&sep1,&end)==6 && sep=='%' &&
                  sep0=='%' && sep1=='%')) &&
                nb_levels>=0.5) { nb_levels = cimg::round(nb_levels); ++position; }
            else { nb_levels = 256; value0 = 0; value1 = 100; sep = 0; sep0 = sep1 = '%'; }
            if (no_min_max) { value0 = 0; value1 = 100; sep0 = sep1 = '%'; }
            print(images,0,"Compute histogram of image%s, using %g%s level%s in range [%g%s,%g%s].",
                  gmic_selection.data(),
                  nb_levels,sep=='%'?"%":"",
                  nb_levels>1?"s":"",
                  value0,sep0=='%'?"%":"",
                  value1,sep1=='%'?"%":"");
            cimg_forY(selection,l) {
              CImg<T> &img = gmic_check(images[selection[l]]);
              nvalue0 = value0; nvalue1 = value1;
              vmin = vmax = 0;
              if (sep0=='%' || sep1=='%') {
                if (img) vmax = (double)img.max_min(vmin);
                if (sep0=='%') nvalue0 = vmin + (vmax - vmin)*value0/100;
                if (sep1=='%') nvalue1 = vmin + (vmax - vmin)*value1/100;
              }
              const unsigned int
                _nb_levels = cimg::max(1U,
                                       (unsigned int)cimg::round(sep=='%'?
                                                                 nb_levels*(1 + nvalue1 - nvalue0)/100:
                                                                 nb_levels));
              gmic_apply(histogram(_nb_levels,(T)nvalue0,(T)nvalue1));
            }
            is_released = false; continue;
          }

          // HSI to RGB.
          gmic_simple_command("-hsi2rgb",HSItoRGB,"Convert image%s from HSI to RGB color bases.");

          // HSL to RGB.
          gmic_simple_command("-hsl2rgb",HSLtoRGB,"Convert image%s from HSL to RGB color bases.");

          // HSV to RGB.
          gmic_simple_command("-hsv2rgb",HSVtoRGB,"Convert image%s from HSV to RGB color bases.");

          // Compute Hessian.
          if (!std::strcmp("-hessian",command)) {
            gmic_substitute_args();
            *argx = 0;
            if (cimg_sscanf(argument,"%255[xyz]%c",
                            argx,&end)==1) {
              ++position;
              print(images,0,"Compute Hessian of image%s along axes '%s'.",
                    gmic_selection.data(),
                    argx);
            } else
              print(images,0,"Compute Hessian of image%s.",
                    gmic_selection.data());
            unsigned int off = 0;
            cimg_forY(selection,l) {
              const unsigned int uind = selection[l] + off;
              CImg<T>& img = gmic_check(images[uind]);
              g_list = img.get_hessian(*argx?argx:0);
              name = images_names[uind];
              if (is_double_hyphen) {
                images_names.insert(g_list.size(),name.copymark());
                g_list.move_to(images,~0U);
              } else {
                off+=g_list.size() - 1;
                g_list[0].move_to(images[uind].assign());
                for (unsigned int i = 1; i<g_list.size(); ++i) g_list[i].move_to(images,uind + i);
                images_names[uind] = name;
                if (g_list.size()>1) images_names.insert(g_list.size() - 1,name.copymark(),uind + 1);
              }
            }
            g_list.assign();
            is_released = false; continue;
          }

        } // command1=='h'.

        //----------------------------
        // Commands starting by '-i..'
        //----------------------------
        else if (command1=='i' && !(command[2]=='f' && !command[3])) {   // (Skip for '-if').

          // Draw image.
          if (!std::strcmp("-image",command)) {
            gmic_substitute_args();
            name.assign(256);
            float x = 0, y = 0, z = 0, c = 0, max_opacity_mask = 1;
            *indices = *name = *argx = *argy = *argz = *argc = sep = sepx = sepy = sepz = sepc = 0;
            ind0.assign();
            opacity = 1;
            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                              indices,&sep,&end)==2 && sep==']') ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-]%c",
                             indices,argx,&end)==2 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             indices,argx,argy,&end)==3 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-]%c",
                             indices,argx,argy,argz,&end)==4 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-]%c",
                             indices,argx,argy,argz,argc,&end)==5 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-],%255[0-9.eE%+-],%f%c",
                             indices,argx,argy,argz,argc,&opacity,&end)==6 ||
                 (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                              "%255[0-9.eE%+-],%255[0-9.eE%+-],%f,[%255[a-zA-Z0-9_.%+-]%c%c",
                              indices,argx,argy,argz,argc,&opacity,name.data(),&sep,&end)==8 &&
                  sep==']') ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-],%255[0-9.eE%+-],%f,[%255[a-zA-Z0-9_.%+-]],%f%c",
                             indices,argx,argy,argz,argc,&opacity,name.data(),
                             &max_opacity_mask,&end)==8) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-image",true,
                                    false,CImg<char>::empty())).height()==1 &&
                (!*name ||
                 (ind0=selection2cimg(name,images.size(),images_names,"-image",true,
                                        false,CImg<char>::empty())).height()==1) &&
                (!*argx ||
                 cimg_sscanf(argx,"%f%c",&x,&end)==1 ||
                 (cimg_sscanf(argx,"%f%c%c",&x,&sepx,&end)==2 && sepx=='%')) &&
                (!*argy ||
                 cimg_sscanf(argy,"%f%c",&y,&end)==1 ||
                 (cimg_sscanf(argy,"%f%c%c",&y,&sepy,&end)==2 && sepy=='%')) &&
                (!*argz ||
                 cimg_sscanf(argz,"%f%c",&z,&end)==1 ||
                 (cimg_sscanf(argz,"%f%c%c",&z,&sepz,&end)==2 && sepz=='%')) &&
                (!*argc ||
                 cimg_sscanf(argc,"%f%c",&c,&end)==1 ||
                 (cimg_sscanf(argc,"%f%c%c",&c,&sepc,&end)==2 && sepc=='%'))) {
              const CImg<T> sprite = gmic_image_arg(*ind);
              CImg<T> mask;
              if (ind0) {
                mask = gmic_image_arg(*ind0);
                print(images,0,"Draw image [%u] at (%g%s,%g%s,%g%s,%g%s) on image%s, "
                      "with opacity %g and mask [%u].",
                      *ind,
                      x,sepx=='%'?"%":"",
                      y,sepy=='%'?"%":"",
                      z,sepz=='%'?"%":"",
                      c,sepc=='%'?"%":"",
                      gmic_selection.data(),
                      opacity,
                      *ind0);
              } else print(images,0,"Draw image [%u] at (%g%s,%g%s,%g%s,%g%s) on image%s, "
                           "with opacity %g.",
                           *ind,
                           x,sepx=='%'?"%":"",
                           y,sepy=='%'?"%":"",
                           z,sepz=='%'?"%":"",
                           c,sepc=='%'?"%":"",
                           gmic_selection.data(),
                           opacity);
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                const int
                  nx = (int)cimg::round(sepx=='%'?x*(img.width() - 1)/100:x),
                  ny = (int)cimg::round(sepy=='%'?y*(img.height() - 1)/100:y),
                  nz = (int)cimg::round(sepz=='%'?z*(img.depth() - 1)/100:z),
                  nc = (int)cimg::round(sepc=='%'?c*(img.spectrum() - 1)/100:c);
                if (ind0) { gmic_apply(draw_image(nx,ny,nz,nc,sprite,mask,opacity,max_opacity_mask)); }
                else gmic_apply(draw_image(nx,ny,nz,nc,sprite,opacity));
              }
            } else arg_error("image");
            is_released = false; ++position; continue;
          }

          // Index image with a LUT.
          if (!std::strcmp("-index",command)) {
            gmic_substitute_args();
            unsigned int lut_type = 0, map_indexes = 0;
            float dithering = 0;
            sep = 0;
            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                            indices,&sep,&end)==2 && sep==']') ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f%c",
                             indices,&dithering,&end)==2 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%u%c",
                             indices,&dithering,&map_indexes,&end)==3) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-index",true,
                                    false,CImg<char>::empty())).height()==1) {
              const float ndithering = dithering<0?0:dithering>1?1:dithering;
              print(images,0,"Index values in image%s by LUT [%u], with dithering level %g%s.",
                    gmic_selection.data(),
                    *ind,
                    ndithering,
                    map_indexes?" and index mapping":"");
              const CImg<T> palette = gmic_image_arg(*ind);
              cimg_forY(selection,l) gmic_apply(index(palette,ndithering,(bool)map_indexes));
            } else if ((cimg_sscanf(argument,"%u%c",&lut_type,&end)==1 ||
                        cimg_sscanf(argument,"%u,%f%c",&lut_type,&dithering,&end)==2 ||
                        cimg_sscanf(argument,"%u,%f,%u%c",
                                    &lut_type,&dithering,&map_indexes,&end)==3) &&
                       lut_type<=7) {
              const float ndithering = dithering<0?0:dithering>1?1:dithering;
              print(images,0,"Index values in image%s by %s color LUT, with dithering level %g%s.",
                    gmic_selection.data(),
                    lut_type==0?"default":lut_type==1?"HSV":lut_type==2?"lines":lut_type==3?"hot":
                    lut_type==4?"cool":lut_type==5?"jet":lut_type==6?"flag":"cube",
                    ndithering,map_indexes?" and index mapping":"");
              const CImg<T>
                palette = lut_type==0?CImg<T>::default_LUT256():lut_type==1?CImg<T>::HSV_LUT256():
                lut_type==2?CImg<T>::lines_LUT256():lut_type==3?CImg<T>::hot_LUT256():
                lut_type==4?CImg<T>::cool_LUT256():lut_type==5?CImg<T>::jet_LUT256():
                lut_type==6?CImg<T>::flag_LUT256():CImg<T>::cube_LUT256();
              cimg_forY(selection,l) gmic_apply(index(palette,ndithering,(bool)map_indexes));
            } else arg_error("index");
            is_released = false; ++position; continue;
          }

          // Matrix inverse.
          gmic_simple_command("-invert",invert,"Invert matrix image%s.");

          // Extract 3d isoline.
          if (!std::strcmp("-isoline3d",command)) {
            gmic_substitute_args();
            float x0 = -3, y0 = -3, x1 = 3, y1 = 3, dx = 256, dy = 256;
            sep = sepx = sepy = *formula = 0;
            value = 0;
            if (cimg_sscanf(argument,"%lf%c",
                            &value,&end)==1 ||
                cimg_sscanf(argument,"%lf%c%c",
                            &value,&sep,&end)==2) {
              print(images,0,"Extract 3d isolines from image%s, using isovalue %g%s.",
                    gmic_selection.data(),
                    value,sep=='%'?"%":"");
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                CImg<T>& img = gmic_check(images[uind]);
                if (img) {
                  vertices.assign();
                  primitives.assign();
                  g_list_uc.assign();
                  g_img_uc.assign(3,img.spectrum(),1,1,220).noise(35,1);
                  if (img.spectrum()==1) g_img_uc(0) = g_img_uc(1) = g_img_uc(2) = 200;
                  else {
                    g_img_uc(0,0) = 255; g_img_uc(1,0) = g_img_uc(2,0) = 30;
                    g_img_uc(0,1) = g_img_uc(2,1) = 30; g_img_uc(1,1) = 255;
                    if (img.spectrum()>=3) g_img_uc(0,2) = g_img_uc(1,2) = 30; g_img_uc(2,2) = 255;
                  }
                  cimg_forC(img,k) {
                    const CImg<T> channel = img.get_shared_channel(k);
                    nvalue = value;
                    if (sep=='%') {
                      vmax = (double)channel.max_min(vmin);
                      nvalue = vmin + (vmax - vmin)*value/100;
                    }
                    CImgList<unsigned int> prims;
                    const CImg<float> pts = img.get_shared_channel(k).get_isoline3d(prims,(float)nvalue);
                    vertices.append_object3d(primitives,pts,prims);
                    g_list_uc.insert(prims.size(),CImg<unsigned char>::vector(g_img_uc(0,k),
                                                                              g_img_uc(1,k),
                                                                              g_img_uc(2,k)));
                  }
                  if (!vertices)
                    warn(images,0,false,
                         "Command '-isoline3d': Isovalue %g%s not found in image [%u].",
                         value,sep=='%'?"%":"",uind);
                  vertices.object3dtoCImg3d(primitives,g_list_uc,false);
                  gmic_apply(replace(vertices));
                  primitives.assign();
                  g_list_uc.assign();
                  g_img_uc.assign();
                } else gmic_apply(replace(img));
              }
            } else if ((cimg_sscanf(argument,"'%4095[^']',%lf%c",
                                    formula,&value,&end)==2 ||
                        cimg_sscanf(argument,"'%4095[^']',%lf,%f,%f,%f,%f%c",
                                    formula,&value,&x0,&y0,&x1,&y1,&end)==6 ||
                        cimg_sscanf(argument,"'%4095[^']',%lf,%f,%f,%f,%f,%f,%f%c",
                                    formula,&value,&x0,&y0,&x1,&y1,&dx,&dy,&end)==8 ||
                        (cimg_sscanf(argument,"'%4095[^']',%lf,%f,%f,%f,%f,%f%c,%f%c",
                                     formula,&value,&x0,&y0,&x1,&y1,&dx,&sepx,&dy,&end)==9 &&
                         sepx=='%') ||
                        (cimg_sscanf(argument,"'%4095[^']',%lf,%f,%f,%f,%f,%f,%f%c%c",
                                     formula,&value,&x0,&y0,&x1,&y1,&dx,&dy,&sepy,&end)==9 &&
                         sepy=='%') ||
                        (cimg_sscanf(argument,"'%4095[^']',%lf,%f,%f,%f,%f,%f%c,%f%c%c",
                                     formula,&value,&x0,&y0,&x1,&y1,&dx,&sepx,&dy,&sepy,&end)==10&&
                         sepx=='%' && sepy=='%')) &&
                       dx>0 && dy>0) {
              dx = cimg::round(dx);
              dy = cimg::round(dy);
              strreplace_fw(formula);
              print(images,0,"Extract 3d isoline %g from formula '%s', in range (%g,%g)-(%g,%g) "
                    "with size %g%sx%g%s.",
                    value,
                    formula,
                    x0,y0,
                    x1,y1,
                    dx,sepx=='%'?"%":"",
                    dy,sepy=='%'?"%":"");
              if (sepx=='%') dx = -dx;
              if (sepy=='%') dy = -dy;
              CImg<T>::isoline3d(primitives,(const char*)formula,(float)value,
                                 x0,y0,x1,y1,(int)dx,(int)dy).move_to(vertices);
              vertices.object3dtoCImg3d(primitives,false).move_to(images);
              primitives.assign();
              cimg_snprintf(title,_title.width(),"[3d isoline %g of '%s']",value,formula);
              CImg<char>::string(title).move_to(images_names);
            } else arg_error("isoline3d");
            is_released = false; ++position; continue;
          }

          // Extract 3d isosurface.
          if (!std::strcmp("-isosurface3d",command)) {
            gmic_substitute_args();
            float x0 = -3, y0 = -3, z0 = -3, x1 = 3, y1 = 3, z1 = 3,
              dx = 32, dy = 32, dz = 32;
            sep = sepx = sepy = sepz = *formula = 0;
            value = 0;
            if (cimg_sscanf(argument,"%lf%c",
                            &value,&end)==1 ||
                cimg_sscanf(argument,"%lf%c%c",
                            &value,&sep,&end)==2) {
              print(images,0,"Extract 3d isosurface from image%s, using isovalue %g%s.",
                    gmic_selection.data(),
                    value,sep=='%'?"%":"");
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                CImg<T>& img = gmic_check(images[uind]);
                if (img) {
                  vertices.assign();
                  primitives.assign();
                  g_list_uc.assign();
                  g_img_uc.assign(3,img.spectrum(),1,1,220).noise(35,1);
                  if (img.spectrum()==1) g_img_uc(0) = g_img_uc(1) = g_img_uc(2) = 200;
                  else {
                    g_img_uc(0,0) = 255; g_img_uc(1,0) = g_img_uc(2,0) = 30;
                    g_img_uc(0,1) = g_img_uc(2,1) = 30; g_img_uc(1,1) = 255;
                    if (img.spectrum()>=3) g_img_uc(0,2) = g_img_uc(1,2) = 30; g_img_uc(2,2) = 255;
                  }
                  cimg_forC(img,k) {
                    const CImg<T> channel = img.get_shared_channel(k);
                    nvalue = value;
                    if (sep=='%') {
                      vmax = (double)channel.max_min(vmin);
                      nvalue = vmin + (vmax - vmin)*value/100;
                    }
                    CImgList<unsigned int> prims;
                    const CImg<float> pts = channel.get_isosurface3d(prims,(float)nvalue);
                    vertices.append_object3d(primitives,pts,prims);
                    g_list_uc.insert(prims.size(),CImg<unsigned char>::vector(g_img_uc(0,k),
                                                                              g_img_uc(1,k),
                                                                              g_img_uc(2,k)));
                  }
                  if (!vertices) {
                    if (img.depth()>1)
                      warn(images,0,false,
                           "Command '-isosurface3d': Isovalue %g%s not found in image [%u].",
                           value,sep=='%'?"%":"",uind);
                    else
                      warn(images,0,false,
                           "Command '-isosurface3d': Image [%u] has a single slice, "
                           "isovalue %g%s not found.",
                           uind,value,sep=='%'?"%":"");
                  }
                  vertices.object3dtoCImg3d(primitives,g_list_uc,false);
                  gmic_apply(replace(vertices));
                  primitives.assign(); g_list_uc.assign(); g_img_uc.assign();
                } else gmic_apply(replace(img));
              }
            } else if ((cimg_sscanf(argument,"'%4095[^']',%lf%c",
                                    formula,&value,&end)==2 ||
                        cimg_sscanf(argument,"'%4095[^']',%lf,%f,%f,%f,%f,%f,%f%c",
                                    formula,&value,&x0,&y0,&z0,&x1,&y1,&z1,&end)==8 ||
                        cimg_sscanf(argument,"'%4095[^']',%lf,%f,%f,%f,%f,%f,%f,%f,%f,%f%c",
                                    formula,&value,&x0,&y0,&z0,&x1,&y1,&z1,&dx,&dy,&dz,&end)==11 ||
                        (cimg_sscanf(argument,"'%4095[^']',%lf,%f,%f,%f,%f,%f,%f,%f%c,%f,%f%c",
                                     formula,&value,&x0,&y0,&z0,&x1,&y1,&z1,
                                     &dx,&sepx,&dy,&dz,&end)==12 &&
                         sepx=='%') ||
                        (cimg_sscanf(argument,"'%4095[^']',%lf,%f,%f,%f,%f,%f,%f,%f,%f%c,%f%c",
                                     formula,&value,&x0,&y0,&z0,&x1,&y1,&z1,
                                     &dx,&dy,&sepy,&dz,&end)==12 &&
                         sepy=='%') ||
                        (cimg_sscanf(argument,"'%4095[^']',%lf,%f,%f,%f,%f,%f,%f,%f,%f,%f%c%c",
                                     formula,&value,&x0,&y0,&z0,&x1,&y1,&z1,
                                     &dx,&dy,&dz,&sepz,&end)==12 &&
                         sepz=='%') ||
                        (cimg_sscanf(argument,"'%4095[^']',%lf,%f,%f,%f,%f,%f,%f,%f%c,%f%c,%f%c",
                                     formula,&value,&x0,&y0,&z0,&x1,&y1,&z1,
                                     &dx,&sepx,&dy,&sepy,&dz,&end)==13 &&
                         sepx=='%' && sepy=='%') ||
                        (cimg_sscanf(argument,"'%4095[^']',%lf,%f,%f,%f,%f,%f,%f,%f%c,%f,%f%c%c",
                                     formula,&value,&x0,&y0,&z0,&x1,&y1,&z1,
                                     &dx,&sepx,&dy,&dz,&sepz,&end)==13 &&
                         sepx=='%' && sepz=='%') ||
                        (cimg_sscanf(argument,"'%4095[^']',%lf,%f,%f,%f,%f,%f,%f,%f,%f%c,%f%c%c",
                                     formula,&value,&x0,&y0,&z0,&x1,&y1,&z1,
                                     &dx,&dy,&sepy,&dz,&sepz,&end)==13 &&
                         sepy=='%' && sepz=='%') ||
                        (cimg_sscanf(argument,"'%4095[^']',%lf,%f,%f,%f,%f,%f,%f,%f%c,%f%c,%f%c%c",
                                     formula,&value,&x0,&y0,&z0,&x1,&y1,&z1,
                                     &dx,&sepx,&dy,&sepy,&dz,&sepz,&end)==14 &&
                         sepx=='%' && sepy=='%' && sepz=='%')) &&
                       dx>0 && dy>0 && dz>0) {
              dx = cimg::round(dx);
              dy = cimg::round(dy);
              dz = cimg::round(dz);
              strreplace_fw(formula);
              print(images,0,"Extract 3d isosurface %g from formula '%s', "
                    "in range (%g,%g,%g)-(%g,%g,%g) with size %g%sx%g%sx%g%s.",
                    value,
                    formula,
                    x0,y0,z0,
                    x1,y1,z1,
                    dx,sepx=='%'?"%":"",
                    dy,sepy=='%'?"%":"",
                    dz,sepz=='%'?"%":"");
              if (sepx=='%') dx = -dx;
              if (sepy=='%') dy = -dy;
              if (sepz=='%') dz = -dz;
              CImg<T>::isosurface3d(primitives,(const char*)formula,(float)value,
                                    x0,y0,z0,x1,y1,z1,(int)dx,(int)dy,(int)dz).move_to(vertices);
              vertices.object3dtoCImg3d(primitives,false).move_to(images);
              primitives.assign();
              cimg_snprintf(title,_title.width(),"[3d isosurface %g of '%s']",value,formula);
              CImg<char>::string(title).move_to(images_names);
            } else arg_error("isosurface3d");
            is_released = false; ++position; continue;
          }

          // Inpaint.
          if (!std::strcmp("-inpaint",command)) {
            gmic_substitute_args();
            float patch_size = 11, lookup_size = 22, lookup_factor = 0.5, lookup_increment = 1,
              blend_size = 0, blend_threshold = 0, blend_decay = 0.05f, blend_scales = 10;
            unsigned int is_blend_outer = 1, method = 1;
            sep = 0;
            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep,&end)==2 &&
                  sep==']') ||
                 (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%c%c",indices,&sep,&end)==2 &&
                  sep=='0') ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],0,%u%c",indices,&method,&end)==2) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-inpaint",true,
                                    false,CImg<char>::empty())).height()==1 &&
                method<=3) {
              print(images,0,"Inpaint image%s masked by image [%u], with %s algorithm.",
                    gmic_selection.data(),
                    *ind,
                    method==0?"low-connectivity average":method==1?"high-connectivity average":
                    method==2?"low-connectivity median":"high-connectivity median");
              const CImg<T> mask = gmic_image_arg(*ind);
              cimg_forY(selection,l) gmic_apply(inpaint(mask,method));
            } else if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                                     indices,&sep,&end)==2 && sep==']') ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f%c",
                                    indices,&patch_size,&end)==2 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f%c",
                                    indices,&patch_size,&lookup_size,&end)==3 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f%c",
                                    indices,&patch_size,&lookup_size,&lookup_factor,&end)==4 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f%c",
                                    indices,&patch_size,&lookup_size,&lookup_factor,
                                    &lookup_increment,&end)==5 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f,%f%c",
                                    indices,&patch_size,&lookup_size,&lookup_factor,
                                    &lookup_increment,&blend_size,&end)==6 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f,%f,%f%c",
                                    indices,&patch_size,&lookup_size,&lookup_factor,
                                    &lookup_increment,&blend_size,&blend_threshold,&end)==7 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f,%f,%f,%f%c",
                                    indices,&patch_size,&lookup_size,&lookup_factor,
                                    &lookup_increment,&blend_size,&blend_threshold,&blend_decay,
                                    &end)==8 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f,%f,%f,%f,%f%c",
                                    indices,&patch_size,&lookup_size,&lookup_factor,
                                    &lookup_increment,&blend_size,&blend_threshold,&blend_decay,
                                    &blend_scales,&end)==9 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f,%f,%f,%f,%f,%u%c",
                                    indices,&patch_size,&lookup_size,&lookup_factor,
                                    &lookup_increment,&blend_size,&blend_threshold,&blend_decay,
                                    &blend_scales,&is_blend_outer,&end)==10) &&
                       (ind=selection2cimg(indices,images.size(),images_names,"-inpaint",true,
                                           false,CImg<char>::empty())).height()==1 &&
                       patch_size>=0.5 && lookup_size>=0.5 && lookup_factor>=0 &&
                       blend_size>=0 && blend_threshold>=0 && blend_threshold<=1 &&
                       blend_decay>=0 && blend_scales>=0.5 && is_blend_outer<=1) {
              const CImg<T> mask = gmic_image_arg(*ind);
              patch_size = cimg::round(patch_size);
              lookup_size = cimg::round(lookup_size);
              lookup_increment = cimg::round(lookup_increment);
              blend_size = cimg::round(blend_size);
              blend_scales = cimg::round(blend_scales);
              print(images,0,"Inpaint image%s masked by image [%d], with patch size %g, "
                    "lookup size %g, lookup factor %g, lookup_increment %g, blend size %g, "
                    "blend threshold %g, blend decay %g, %g blend scale%s and outer blending %s.",
                    gmic_selection.data(),*ind,
                    patch_size,lookup_size,lookup_factor,lookup_increment,
                    blend_size,blend_threshold,blend_decay,blend_scales,blend_scales!=1?"s":"",
                    is_blend_outer?"enabled":"disabled");
              cimg_forY(selection,l)
                gmic_apply(inpaint_patch(mask,
                                         (unsigned int)patch_size,(unsigned int)lookup_size,
                                         lookup_factor,
                                         (int)lookup_increment,
                                         (unsigned int)blend_size,blend_threshold,blend_decay,
                                         (unsigned int)blend_scales,(bool)is_blend_outer));
            } else arg_error("inpaint");
            is_released = false; ++position; continue;
          }

        } // command1=='i'.

        //----------------------------
        // Commands starting by '-k..'
        //----------------------------
        else if (command1=='k') {

          // Keep images.
          if (!std::strcmp("-keep",command)) {
            print(images,0,"Keep image%s",
                  gmic_selection.data());
            g_list.assign(selection.height());
            g_list_c.assign(selection.height());
            if (is_double_hyphen) {
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                g_list[l].assign(images[uind]);
                g_list_c[l].assign(images_names[uind]).copymark();
              }
              g_list.move_to(images,~0U);
              g_list_c.move_to(images_names,~0U);
            } else {
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                g_list[l].swap(images[uind]);
                g_list_c[l].swap(images_names[uind]);
              }
              g_list.swap(images);
              g_list_c.swap(images_names);
            }
            if (is_verbose) {
              cimg::mutex(29);
              std::fprintf(cimg::output()," (%u image%s left).",
                           images.size(),images.size()==1?"":"s");
              std::fflush(cimg::output());
              cimg::mutex(29,0);
            }
            g_list.assign(); g_list_c.assign();
            is_released = false; continue;
          }

        } // command1=='k'.

        //----------------------------
        // Commands starting by '-l..'
        //----------------------------
        else if (command1=='l') {

          // Start local environnement.
          if (!std::strcmp("-local",command)) {
            if (is_debug_info && debug_line!=~0U) {
              cimg_snprintf(argx,_argx.width(),"*local#%u",debug_line);
              CImg<char>::string(argx).move_to(callstack);
            } else CImg<char>::string("*local").move_to(callstack);
            if (is_very_verbose)
              print(images,0,"Start '-local..-endlocal' block, with selected image%s.",
                    gmic_selection.data());
            g_list.assign(selection.height());
            g_list_c.assign(selection.height());
            gmic_exception exception;

            if (is_double_hyphen) cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                g_list[l].assign(images[uind]);
                g_list_c[l].assign(images_names[uind]).copymark();
              } else {
                cimg::mutex(27);
                cimg_forY(selection,l) {
                  const unsigned int uind = selection[l];
                  if (images[uind].is_shared())
                    g_list[l].assign(images[uind],false);
                  else {
                    if ((images[uind]._width || images[uind]._height) && !images[uind]._spectrum) {
                      selection2string(selection,images_names,1,name);
                      error(images,0,0,
                            "Command '-local': Invalid selection%s "
                            "(image [%u] is already used in another thread).",
                            name.data() + (*name=='s'?1:0),uind);
                    }
                    g_list[l].swap(images[uind]);
                    // Small hack to be able to track images of the selection passed to the new environment.
                    std::memcpy(&images[uind]._width,&g_list[l]._data,sizeof(void*));
                    images[uind]._spectrum = 0;
                  }
                  g_list_c[l] = images_names[uind];   // Make a copy to be still able to recognize '-pass[label]'.
                }
                cimg::mutex(27,0);
            }
            const unsigned int local_callstack_size = callstack.size();
            try {
              if (next_debug_line!=~0U) { debug_line = next_debug_line; next_debug_line = ~0U; }
              if (next_debug_filename!=~0U) { debug_filename = next_debug_filename; next_debug_filename = ~0U; }
              _run(commands_line,position,g_list,g_list_c,images,images_names,variables_sizes,is_noarg,0,
                   command_selection);
            } catch (gmic_exception &e) {
              int nb_locals = 0;
              for (nb_locals = 1; nb_locals && position<commands_line.size(); ++position) {
                const char *const it = commands_line[position].data();
                if (*it==1 &&
                    cimg_sscanf(commands_line[position].data() + 1,"%x,%x",&_debug_line,&(_debug_filename=0))>0) {
                  is_debug_info = true; next_debug_line = _debug_line; next_debug_filename = _debug_filename;
                } else if (!std::strcmp("-local",it) || !std::strcmp("-l",it) ||
                           !std::strcmp("--local",it) || !std::strcmp("--l",it) ||
                           !std::strncmp("-local[",it,7) || !std::strncmp("-l[",it,3) ||
                           !std::strncmp("--local[",it,8) || !std::strncmp("--l[",it,4)) ++nb_locals;
                else if (!std::strcmp("-endlocal",it) || !std::strcmp("-endl",it)) --nb_locals;
                else if (nb_locals==1 && !std::strcmp("-onfail",it)) break;
              }
              if (callstack.size()>local_callstack_size) callstack.remove(local_callstack_size,callstack.size() - 1);
              if (nb_locals==1 && position<commands_line.size()) { // Onfail block found.
                if (is_very_verbose) print(images,0,"Reach '-onfail' block.");
                try {
                  _run(commands_line,++position,g_list,g_list_c,
                       parent_images,parent_images_names,variables_sizes,is_noarg,0,0);
                } catch (gmic_exception &e) {
                  cimg::swap(exception._command_help,e._command_help);
                  cimg::swap(exception._message,e._message);
                }
              } else {
                cimg::swap(exception._command_help,e._command_help);
                cimg::swap(exception._message,e._message);
              }
            }
            callstack.remove();
            if (is_double_hyphen) {
              g_list.move_to(images,~0U);
              g_list_c.move_to(images_names,~0U);
            } else {
              const unsigned int nb = cimg::min((unsigned int)selection.height(),g_list.size());
              if (nb>0) {
                for (unsigned int i = 0; i<nb; ++i) {
                  const unsigned int uind = selection[i];
                  if (images[uind].is_shared()) {
                    images[uind] = g_list[i];
                    g_list[i].assign();
                  } else images[uind].swap(g_list[i]);
                  images_names[uind].swap(g_list_c[i]);
                }
                g_list.remove(0,nb - 1);
                g_list_c.remove(0,nb - 1);
              }
              if (nb<(unsigned int)selection.height())
                remove_images(images,images_names,selection,nb,selection.height() - 1);
              else if (g_list) {
                const unsigned int uind0 = selection?selection.back() + 1:images.size();
                images.insert(g_list,uind0);
                g_list_c.move_to(images_names,uind0);
              }
            }
            if (exception._message) throw exception;
            g_list.assign(); g_list_c.assign();
            continue;
          }

          // Less or equal.
          gmic_arithmetic_command("-le",
                                  operator_le,"Compute boolean 'less or equal than' between image%s "
                                  "and %g%s",
                                  gmic_selection.data(),value,ssep,T,
                                  operator_le,"Compute boolean 'less or equal than' between image%s "
                                  "and image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute boolean 'less or equal than' between image%s and "
                                  "expression %s'",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute boolean 'less or equal than' between image%s");

          // Less than.
          gmic_arithmetic_command("-lt",
                                  operator_lt,
                                  "Compute boolean 'less than' between image%s and %g%s",
                                  gmic_selection.data(),value,ssep,T,
                                  operator_lt,
                                  "Compute boolean 'less than' between image%s and image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute boolean 'less than' between image%s and expression %s'",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute boolean 'less than' between image%s");

          // Logarithm, base-e.
          gmic_simple_command("-log",log,"Compute pointwise base-e logarithm of image%s.");

          // Logarithm, base-2.
          gmic_simple_command("-log2",log2,"Compute pointwise base-2 logarithm of image%s.");

          // Logarithm, base-10.
          gmic_simple_command("-log10",log10,"Compute pointwise base-10 logarithm of image%s.");

          // Draw line.
          if (!std::strcmp("-line",command)) {
            gmic_substitute_args();
            *argx = *argy = *argz = *argc = *color = 0;
            float x0 = 0, y0 = 0, x1 = 0, y1 = 0;
            char sepx0 = 0, sepy0 = 0, sepx1 = 0, sepy1 = 0, sep1 = 0;
            pattern = ~0U; opacity = 1;
            if ((cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-]%c",
                             argx,argy,argz,argc,&end)==4 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-],%f%c",
                             argx,argy,argz,argc,&opacity,&end)==5 ||
                 (cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                              "%255[0-9.eE%+-],%f,0%c%x%c",
                              argx,argy,argz,argc,&opacity,&sep1,&pattern,&end)==7 &&
                  sep1=='x') ||
                 (cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                              "%255[0-9.eE%+-],%f,%4095[0-9.eEinfa,+-]%c",
                              argx,argy,argz,argc,&opacity,color,&end)==6 && (bool)(pattern=~0U)) ||
                 (*color=0,cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                                       "%255[0-9.eE%+-],%f,0%c%x,%4095[0-9.eEinfa,+-]%c",
                                       argx,argy,argz,argc,&opacity,&sep1,
                                       &pattern,color,&end)==8 && sep1=='x')) &&
                (cimg_sscanf(argx,"%f%c",&x0,&end)==1 ||
                 (cimg_sscanf(argx,"%f%c%c",&x0,&sepx0,&end)==2 && sepx0=='%')) &&
                (cimg_sscanf(argy,"%f%c",&y0,&end)==1 ||
                 (cimg_sscanf(argy,"%f%c%c",&y0,&sepy0,&end)==2 && sepy0=='%')) &&
                (cimg_sscanf(argz,"%f%c",&x1,&end)==1 ||
                 (cimg_sscanf(argz,"%f%c%c",&x1,&sepx1,&end)==2 && sepx1=='%')) &&
                (cimg_sscanf(argc,"%f%c",&y1,&end)==1 ||
                 (cimg_sscanf(argc,"%f%c%c",&y1,&sepy1,&end)==2 && sepy1=='%'))) {
              print(images,0,"Draw line (%g%s,%g%s) - (%g%s,%g%s) on image%s, with opacity %g, "
                    "pattern 0x%x and color (%s).",
                    x0,sepx0=='%'?"%":"",
                    y0,sepy0=='%'?"%":"",
                    x1,sepx1=='%'?"%":"",
                    y1,sepy1=='%'?"%":"",
                    gmic_selection.data(),
                    opacity,pattern,
                    *color?color:"default");
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                g_img.assign(img.spectrum(),1,1,1,0).fill(color,true,false);
                const int
                  nx0 = (int)cimg::round(sepx0=='%'?x0*(img.width() - 1)/100:x0),
                  ny0 = (int)cimg::round(sepy0=='%'?y0*(img.height() - 1)/100:y0),
                  nx1 = (int)cimg::round(sepx1=='%'?x1*(img.width() - 1)/100:x1),
                  ny1 = (int)cimg::round(sepy1=='%'?y1*(img.height() - 1)/100:y1);
                gmic_apply(draw_line(nx0,ny0,nx1,ny1,g_img.data(),opacity,pattern));
              }
            } else arg_error("line");
            g_img.assign();
            is_released = false; ++position; continue;
          }

          // Lab to RGB
          gmic_simple_command("-lab2rgb",LabtoRGB,"Convert image%s from Lab to RGB color bases.");

          // Label connected components.
          if (!std::strcmp("-label",command)) {
            gmic_substitute_args();
            unsigned int is_high_connectivity = 0;
            float tolerance = 0;
            if ((cimg_sscanf(argument,"%f%c",&tolerance,&end)==1 ||
                 cimg_sscanf(argument,"%f,%u%c",&tolerance,&is_high_connectivity,&end)==2) &&
                tolerance>=0) ++position;
            else { tolerance = 0; is_high_connectivity = 0; }
            print(images,0,
                  "Label connected components on image%s, with tolerance %g and "
                  "%s connectivity.",
                  gmic_selection.data(),tolerance,is_high_connectivity?"high":"low");
            cimg_forY(selection,l) gmic_apply(label((bool)is_high_connectivity,tolerance));
            is_released = false; continue;
          }

          // Set 3d light position.
          if (!std::strcmp("-light3d",item)) {
            gmic_substitute_args();
            float lx = 0, ly = 0, lz = -5e8f;
            sep = 0;
            if (cimg_sscanf(argument,"%f,%f,%f%c",
                            &lx,&ly,&lz,&end)==3) {
              print(images,0,"Set 3d light position to (%g,%g,%g).",
                    lx,ly,lz);
              light3d_x = lx;
              light3d_y = ly;
              light3d_z = lz;
              ++position;
            } else if (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep,&end)==2 &&
                       sep==']' &&
                       (ind=selection2cimg(indices,images.size(),images_names,"-light3d",true,
                                           false,CImg<char>::empty())).height()==1) {
              print(images,0,"Set 3d light texture from image [%u].",*ind);
              light3d.assign(images[*ind],false);
              ++position;
            } else {
              print(images,0,"Reset 3d light to default.");
              light3d.assign();
              light3d_x = light3d_y = 0; light3d_z = -5e8f;
            }
            continue;
          }

        } // command1=='l'.

        //----------------------------
        // Commands starting by '-m..'
        //----------------------------
        else if (command1=='m') {

          // Move images.
          if (!std::strcmp("-move",command)) {
            gmic_substitute_args();
            float pos = 0;
            sep = 0;
            if (cimg_sscanf(argument,"%f%c",&pos,&end)==1 ||
                (cimg_sscanf(argument,"%f%c%c",&pos,&sep,&end)==2 && sep=='%')) {
              const int
                _iind0 = (int)cimg::round(sep=='%'?pos*images.size()/100:pos),
                iind0 = _iind0<0?_iind0 + (int)images.size():_iind0;
              if (iind0<0 || iind0>(int)images.size())
                error(images,0,0,
                      "Command '-move': Invalid position '%d' (not in range -%u...%u).",
                      _iind0,images.size(),images.size() - 1);
              print(images,0,"Move image%s to position %d.",
                    gmic_selection.data(),
                    iind0);
              CImgList<T> _images, nimages;
              CImgList<char> _images_names, nimages_names;
              if (is_double_hyphen) {
                _images.insert(images.size());
                // Copy original list while preserving shared state of each item.
                cimglist_for(_images,l) _images[l].assign(images[l],images[l].is_shared());
                _images_names.assign(images_names);
              }
              nimages.insert(selection.height());
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                if (is_double_hyphen) images[uind].move_to(nimages[l]);
                else images[uind].swap(nimages[l]);
                // Empty shared image as a special item to be removed later.
                images[uind]._is_shared = true;
                images_names[uind].move_to(nimages_names);
              }
              images.insert(nimages.size(),iind0);
              cimglist_for(nimages,l) nimages[l].swap(images[iind0 + l]);
              nimages_names.move_to(images_names,iind0);
              cimglist_for(images,l) if (!images[l] && images[l].is_shared()) {
                images.remove(l); images_names.remove(l--); // Remove special items.
              }
              if (is_double_hyphen) {
                cimglist_for(images,l) // Replace shared items by non-shared one for a get version.
                  if (images[l].is_shared()) {
                    CImg<T> tmp; (images[l].move_to(tmp)).swap(images[l]);
                  }
                images.insert(_images.size(),0);
                cimglist_for(_images,l) images[l].swap(_images[l]);
                _images_names.move_to(images_names,0);
              }
            } else arg_error("move");
            is_released = false; ++position; continue;
          }

          // Mirror.
          if (!std::strcmp("-mirror",command)) {
            gmic_substitute_args();
            bool is_valid_argument = *argument!=0;
            if (is_valid_argument) for (const char *s = argument; *s; ++s) {
                const char _s = *s;
                if (_s!='x' && _s!='y' && _s!='z' && _s!='c') { is_valid_argument = false; break; }
              }
            if (is_valid_argument) {
              print(images,0,"Mirror image%s along the '%s'-ax%cs.",
                    gmic_selection.data(),
                    gmic_argument_text_printed(),
                    std::strlen(argument)>1?'e':'i');
              cimg_forY(selection,l) gmic_apply(mirror(argument));
            } else arg_error("mirror");
            is_released = false; ++position; continue;
          }

          // Manage mutexes.
          if (!std::strcmp("-mutex",item)) {
            gmic_substitute_args();
            unsigned int number, is_lock = 1;
            if ((cimg_sscanf(argument,"%u%c",
                             &number,&end)==1 ||
                 cimg_sscanf(argument,"%u,%u%c",
                             &number,&is_lock,&end)==2) &&
                number<256 && is_lock<=1) {
              print(images,0,"%s mutex #%u.",
                    is_lock?"Lock":"Unlock",number);
              if (is_lock) gmic_mutex().lock(number);
              else gmic_mutex().unlock(number);
            } else arg_error("mutex");
            ++position; continue;
          }

          // Multiplication.
          gmic_arithmetic_command("-mul",
                                  operator*=,
                                  "Multiply image%s by %g%s",
                                  gmic_selection.data(),value,ssep,Tfloat,
                                  mul,
                                  "Multiply image%s by image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Multiply image%s by expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Multiply image%s");
          // Modulo.
          gmic_arithmetic_command("-mod",
                                  operator%=,
                                  "Compute pointwise modulo of image%s by %g%s",
                                  gmic_selection.data(),value,ssep,T,
                                  operator%=,
                                  "Compute pointwise modulo of image%s by image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute pointwise modulo of image%s by expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute sequential pointwise modulo of image%s");

          // Max.
          gmic_arithmetic_command("-max",
                                  max,
                                  "Compute pointwise maximum between image%s and %g%s",
                                  gmic_selection.data(),value,ssep,T,
                                  max,
                                  "Compute pointwise maximum between image%s and image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute pointwise maximum between image%s and expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute pointwise maximum of all image%s together");
          // Min.
          gmic_arithmetic_command("-min",
                                  min,
                                  "Compute pointwise minimum between image%s and %g%s",
                                  gmic_selection.data(),value,ssep,T,
                                  min,
                                  "Compute pointwise minimum between image%s and image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute pointwise minimum between image%s and expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute pointwise minimum of image%s");

          // Matrix multiplication.
          gmic_arithmetic_command("-mmul",
                                  operator*=,
                                  "Multiply matrix/vector%s by %g%s",
                                  gmic_selection.data(),value,ssep,Tfloat,
                                  operator*=,
                                  "Multiply matrix/vector%s by matrix/vector image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Multiply matrix/vector%s by expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Multiply matrix/vector%s");

          // Set 3d rendering modes.
          if (!std::strcmp("-mode3d",item)) {
            gmic_substitute_args();
            float mode = 4;
            if (cimg_sscanf(argument,"%f%c",
                            &mode,&end)==1 &&
                (mode=cimg::round(mode))>=-1 && mode<=5) ++position;
            else mode = 4;
            render3d = (int)mode;
            print(images,0,"Set static 3d rendering mode to %s.",
                  render3d==-1?"bounding-box":
                  render3d==0?"pointwise":render3d==1?"linear":render3d==2?"flat":
                  render3d==3?"flat-shaded":render3d==4?"Gouraud-shaded":
                  render3d==5?"Phong-shaded":"none");
            continue;
          }

          if (!std::strcmp("-moded3d",item)) {
            gmic_substitute_args();
            float mode = -1;
            if (cimg_sscanf(argument,"%f%c",
                            &mode,&end)==1 &&
                (mode=cimg::round(mode))>=-1 && mode<=5) ++position;
            else mode = -1;
            renderd3d = (int)mode;
            print(images,0,"Set dynamic 3d rendering mode to %s.",
                  renderd3d==-1?"bounding-box":
                  renderd3d==0?"pointwise":renderd3d==1?"linear":renderd3d==2?"flat":
                  renderd3d==3?"flat-shaded":renderd3d==4?"Gouraud-shaded":
                  renderd3d==5?"Phong-shaded":"none");
            continue;
          }

          // Map LUT.
          if (!std::strcmp("-map",command)) {
            gmic_substitute_args();
            unsigned int lut_type = 0;
            boundary = 0;
            sep = 0;
            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep,&end)==2 && sep==']') ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u%c",indices,&boundary,&end)==2) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-map",true,
                                    false,CImg<char>::empty())).height()==1 &&
                boundary<=2) {
              print(images,0,"Map LUT [%u] on image%s, with %s boundary conditions.",
                    *ind,
                    gmic_selection.data(),
                    boundary==0?"dirichlet":boundary==1?"neumann":"periodic");
              const CImg<T> palette = gmic_image_arg(*ind);
              cimg_forY(selection,l) gmic_apply(map(palette,boundary));
            } else if ((cimg_sscanf(argument,"%u%c",&lut_type,&end)==1 ||
                        cimg_sscanf(argument,"%u,%u%c",&lut_type,&boundary,&end)==2) &&
                       lut_type<=7 && boundary<=2) {
              print(images,0,"Map %s color LUT on image%s, with %s boundary conditions.",
                    lut_type==0?"default":lut_type==1?"HSV":lut_type==2?"lines":lut_type==3?"hot":
                    lut_type==4?"cool":lut_type==5?"jet":lut_type==6?"flag":"cube",
                    gmic_selection.data(),
                    boundary==0?"dirichlet":boundary==1?"neumann":"periodic");
              const CImg<T>
                palette = lut_type==0?CImg<T>::default_LUT256():lut_type==1?CImg<T>::HSV_LUT256():
                lut_type==2?CImg<T>::lines_LUT256():lut_type==3?CImg<T>::hot_LUT256():
                lut_type==4?CImg<T>::cool_LUT256():lut_type==5?CImg<T>::jet_LUT256():
                lut_type==6?CImg<T>::flag_LUT256():CImg<T>::cube_LUT256();
              cimg_forY(selection,l) gmic_apply(map(palette,boundary));
            } else arg_error("map");
            is_released = false; ++position; continue;
          }

          // Median filter.
          if (!std::strcmp("-median",command)) {
            gmic_substitute_args();
            float siz = 3, threshold = 0;
            if ((cimg_sscanf(argument,"%f%c",
                            &siz,&end)==1 ||
                 cimg_sscanf(argument,"%f,%f%c",
                             &siz,&threshold,&end)==2) &&
                siz>=0 && threshold>=0) {
              siz = cimg::round(siz);
              if (threshold)
                print(images,0,"Apply median filter of size %g with threshold %g, on image%s.",
                      siz,threshold,
                      gmic_selection.data());
              else
                print(images,0,"Apply median filter of size %g, on image%s.",
                      siz,
                      gmic_selection.data());
              cimg_forY(selection,l) gmic_apply(blur_median((unsigned int)siz,threshold));
            } else arg_error("median");
            is_released = false; ++position; continue;
          }

          // Matrix division.
          gmic_arithmetic_command("-mdiv",
                                  operator/=,
                                  "Divide matrix/vector%s by %g%s",
                                  gmic_selection.data(),value,ssep,Tfloat,
                                  operator/=,
                                  "Divide matrix/vector%s by matrix/vector image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Divide matrix/vector%s by expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Divide matrix/vector%s");

          // MSE.
          if (!std::strcmp("-mse",command)) {
            print(images,0,"Compute the %dx%d matrix of MSE values, from image%s.",
                  selection.height(),selection.height(),
                  gmic_selection.data());
            if (selection) {
              g_list.assign(selection.height());
              cimg_forY(selection,l) g_list[l].assign(gmic_check(images[selection[l]]),true);
              CImg<T> img(g_list.size(),g_list.size(),1,1,(T)0);
              cimg_forXY(img,x,y) if (x>y) img(x,y) = img(y,x) = (T)g_list[x].MSE(g_list[y]);
              CImg<char>::string("[MSE]").move_to(name);
              if (is_double_hyphen) {
                img.move_to(images);
                name.move_to(images_names);
              } else {
                remove_images(images,images_names,selection,1,selection.height() - 1);
                img.move_to(images[selection[0]].assign());
                name.move_to(images_names[selection[0]]);
              }
              g_list.assign();
            }
            is_released = false; continue;
          }

          // Draw mandelbrot/julia fractal.
          if (!std::strcmp("-mandelbrot",command)) {
            gmic_substitute_args();
            double z0r = -2, z0i = -2, z1r = 2, z1i = 2, paramr = 0, parami = 0;
            unsigned int is_julia = 0;
            float itermax = 100;
            opacity = 1;
            if ((cimg_sscanf(argument,"%lf,%lf,%lf,%lf%c",
                             &z0r,&z0i,&z1r,&z1i,&end)==4 ||
                 cimg_sscanf(argument,"%lf,%lf,%lf,%lf,%f%c",
                             &z0r,&z0i,&z1r,&z1i,&itermax,&end)==5 ||
                 cimg_sscanf(argument,"%lf,%lf,%lf,%lf,%f,%u%c",
                             &z0r,&z0i,&z1r,&z1i,&itermax,&is_julia,&end)==6 ||
                 cimg_sscanf(argument,"%lf,%lf,%lf,%lf,%f,%u,%lf,%lf%c",
                             &z0r,&z0i,&z1r,&z1i,&itermax,&is_julia,&paramr,
                             &parami,&end)==8 ||
                 cimg_sscanf(argument,"%lf,%lf,%lf,%lf,%f,%u,%lf,%lf,%f%c",
                             &z0r,&z0i,&z1r,&z1i,&itermax,&is_julia,
                             &paramr,&parami,&opacity,&end)==9) &&
                itermax>=0 && is_julia<=1) {
              itermax = cimg::round(itermax);
              print(images,0,"Draw %s fractal on image%s, from complex area (%g,%g)-(%g,%g) "
                    "with c0 = (%g,%g) and %g iterations.",
                    is_julia?"julia":"mandelbrot",
                    gmic_selection.data(),
                    z0r,z0i,
                    z1r,z1i,
                    paramr,parami,
                    itermax);
              cimg_forY(selection,l) gmic_apply(draw_mandelbrot(CImg<T>(),opacity,z0r,z0i,z1r,z1i,(unsigned int)itermax,
                                                                true,(bool)is_julia,paramr,parami));
            } else arg_error("mandelbrot");
            is_released = false; ++position; continue;
          }

        } // command1=='m'.

        //----------------------------
        // Commands starting by '-n..'
        //----------------------------
        else if (command1=='n') {

          // Set image name.
          if (!is_double_hyphen && !std::strcmp("-name",command)) {
            gmic_substitute_args();
            print(images,0,"Set name of image%s to '%s'.",
                  gmic_selection.data(),gmic_argument_text_printed());
            CImg<char>::string(argument).move_to(name);
            strreplace_fw(name);
            cimg_forY(selection,l) images_names[selection[l]].assign(name);
            ++position; continue;
          }

          // Normalize.
          if (!std::strcmp("-normalize",command)) {
            gmic_substitute_args();
            ind0.assign(); ind1.assign();
            sep0 = sep1 = *argx = *argy = 0;
            value0 = value1 = 0;
            if (cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-]%c",
                            argx,argy,&end)==2 &&
                ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep0,&end)==2 &&
                  sep0==']' &&
                  (ind0=selection2cimg(indices,images.size(),images_names,"-normalize",true,
                                       false,CImg<char>::empty())).height()==1) ||
                 (cimg_sscanf(argx,"%lf%c%c",&value0,&sep0,&end)==2 && sep0=='%') ||
                 cimg_sscanf(argx,"%lf%c",&value0,&end)==1) &&
                ((cimg_sscanf(argy,"[%255[a-zA-Z0-9_.%+-]%c%c",formula,&sep1,&end)==2 &&
                  sep1==']' &&
                  (ind1=selection2cimg(formula,images.size(),images_names,"-normalize",true,
                                       false,CImg<char>::empty())).height()==1) ||
                 (cimg_sscanf(argy,"%lf%c%c",&value1,&sep1,&end)==2 && sep1=='%') ||
                 cimg_sscanf(argy,"%lf%c",&value1,&end)==1)) {
              if (ind0) { value0 = images[*ind0].min(); sep0 = 0; }
              if (ind1) { value1 = images[*ind1].max(); sep1 = 0; }
              print(images,0,"Normalize image%s in range [%g%s,%g%s].",
                    gmic_selection.data(),
                    value0,sep0=='%'?"%":"",
                    value1,sep1=='%'?"%":"");
              cimg_forY(selection,l) {
                CImg<T>& img = gmic_check(images[selection[l]]);
                nvalue0 = value0; nvalue1 = value1;
                vmin = vmax = 0;
                if (sep0=='%' || sep1=='%') {
                  if (img) vmax = (double)img.max_min(vmin);
                  if (sep0=='%') nvalue0 = vmin + (vmax - vmin)*value0/100;
                  if (sep1=='%') nvalue1 = vmin + (vmax - vmin)*value1/100;
                }
                gmic_apply(normalize((T)nvalue0,(T)nvalue1));
              }
            } else if (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep0,&end)==2 &&
                       sep0==']' &&
                       (ind0=selection2cimg(indices,images.size(),images_names,"-normalize",true,
                                            false,CImg<char>::empty())).height()==1) {
              if (images[*ind0]) value1 = (double)images[*ind0].max_min(value0);
              print(images,0,"Normalize image%s in range [%g,%g].",
                    gmic_selection.data(),
                    value0,
                    value1);
              cimg_forY(selection,l) gmic_apply(normalize((T)value0,(T)value1));
            } else arg_error("normalize");
            is_released = false; ++position; continue;
          }

          // Test difference.
          gmic_arithmetic_command("-neq",
                                  operator_neq,
                                  "Compute boolean inequality between image%s and %g%s",
                                  gmic_selection.data(),value,ssep,T,
                                  operator_neq,
                                  "Compute boolean inequality between image%s and image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute boolean inequality between image%s and expression %s'",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute boolean inequality between image%s");

          // Discard custom command arguments.
          if (!std::strcmp("-noarg",item)) {
            print(images,0,"Discard command arguments.");
            if (is_noarg) *is_noarg = true;
            continue;
          }

          // Add noise.
          if (!std::strcmp("-noise",command)) {
            gmic_substitute_args();
            int noise_type = 0;
            float sigma = 0;
            sep = 0;
            if ((cimg_sscanf(argument,"%f%c",
                             &sigma,&end)==1 ||
                 (cimg_sscanf(argument,"%f%c%c",
                              &sigma,&sep,&end)==2 && sep=='%') ||
                 cimg_sscanf(argument,"%f,%d%c",
                             &sigma,&noise_type,&end)==2 ||
                 (cimg_sscanf(argument,"%f%c,%d%c",
                              &sigma,&sep,&noise_type,&end)==3 && sep=='%')) &&
                sigma>=0 && noise_type>=0 && noise_type<=4) {
              const char *s_type = noise_type==0?"gaussian":
                noise_type==1?"uniform":
                noise_type==2?"salt&pepper":
                noise_type==3?"poisson":"rice";
              if (sep=='%') sigma = -sigma;
              print(images,0,"Add %s noise to image%s, with standard deviation %g%s.",
                    s_type,
                    gmic_selection.data(),
                    cimg::abs(sigma),sep=='%'?"%":"");
              cimg_forY(selection,l) gmic_apply(noise(sigma,noise_type));
            } else arg_error("noise");
            is_released = false; ++position; continue;
          }

        } // command1=='n'.

        //----------------------------
        // Commands starting by '-o..'
        //----------------------------
        else if (command1=='o') {

          // Exception handling in local environments.
          if (!std::strcmp("-onfail",item)) {
            const CImg<char> &s = callstack.back();
            if (s[0]!='*' || s[1]!='l')
              error(images,0,0,
                    "Command '-onfail': Not associated to a '-local' command within "
                    "the same scope.");
            for (int nb_locals = 1; nb_locals && position<commands_line.size(); ++position) {
              const char *const it = commands_line[position].data();
              if (*it==1 &&
                  cimg_sscanf(commands_line[position].data() + 1,"%x,%x",&_debug_line,&(_debug_filename=0))>0) {
                is_debug_info = true; next_debug_line = _debug_line; next_debug_filename = _debug_filename;
              } else if (!std::strcmp("-local",it) || !std::strcmp("-l",it) ||
                         !std::strcmp("--local",it) || !std::strcmp("--l",it) ||
                         !std::strncmp("-local[",it,7) || !std::strncmp("-l[",it,3) ||
                         !std::strncmp("--local[",it,8) || !std::strncmp("--l[",it,4)) ++nb_locals;
              else if (!std::strcmp("-endlocal",it) || !std::strcmp("-endl",it)) {
                --nb_locals; if (!nb_locals) --position;
              }
            }
            continue;
          }

          // Draw 3d object.
          if (!std::strcmp("-object3d",command)) {
            gmic_substitute_args();
            float x = 0, y = 0, z = 0;
            unsigned int
              is_zbuffer = 1,
              _render3d = (unsigned int)cimg::max(0,render3d),
              _is_double3d = is_double3d?1U:0U;
            float
              _focale3d = focale3d,
              _light3d_x = light3d_x,
              _light3d_y = light3d_y,
              _light3d_z = light3d_z,
              _specular_lightness3d = specular_lightness3d,
              _specular_shininess3d = specular_shininess3d;
            sep = sepx = sepy = *argx = *argy = 0;
            opacity = 1;
            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                              indices,&sep,&end)==2 && sep==']') ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-]%c",
                             indices,argx,&end)==2 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             indices,argx,argy,&end)==3 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%f%c",
                             indices,argx,argy,&z,&end)==4 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%f,%f%c",
                             indices,argx,argy,&z,&opacity,&end)==5 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%f,%f,%u%c",
                             indices,argx,argy,&z,&opacity,&_render3d,&end)==6 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%f,%f,%u,%u%c",
                             indices,argx,argy,&z,&opacity,&_render3d,&_is_double3d,&end)==7 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%f,%f,%u,%u,%u%c",
                             indices,argx,argy,&z,&opacity,&_render3d,&_is_double3d,&is_zbuffer,
                             &end)==8 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%f,%f,%u,%u,%u,%f%c",
                             indices,argx,argy,&z,&opacity,&_render3d,&_is_double3d,&is_zbuffer,
                             &_focale3d,&end)==9 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%f,%f,%u,%u,%u,%f,%f,%f,%f%c",
                             indices,argx,argy,&z,&opacity,&_render3d,&_is_double3d,&is_zbuffer,
                             &_focale3d,&_light3d_x,&_light3d_y,&_light3d_z,&end)==12 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%f,%f,%u,%u,%u,%f,%f,%f,%f,%f,%f%c",
                             indices,argx,argy,&z,&opacity,&_render3d,&_is_double3d,&is_zbuffer,
                             &_focale3d,&_light3d_x,&_light3d_y,&_light3d_z,
                             &_specular_lightness3d,&_specular_shininess3d,&end)==14) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-object3d",true,
                                    false,CImg<char>::empty())).height()==1 &&
                (!*argx ||
                 cimg_sscanf(argx,"%f%c",&x,&end)==1 ||
                 (cimg_sscanf(argx,"%f%c%c",&x,&sepx,&end)==2 && sepx=='%')) &&
                (!*argy ||
                 cimg_sscanf(argy,"%f%c",&y,&end)==1 ||
                 (cimg_sscanf(argy,"%f%c%c",&y,&sepy,&end)==2 && sepy=='%')) &&
                _render3d<=5 && is_zbuffer<=1 && _is_double3d<=1) {
              const CImg<T> img0 = gmic_image_arg(*ind);

              print(images,0,"Draw 3d object [%u] at (%g%s,%g%s,%g) on image%s, with opacity %g, "
                    "%s rendering, %s-sided mode, %sz-buffer, focale %g, 3d light at (%g,%g,%g) "
                    "and specular properties (%g,%g)",
                    *ind,
                    x,sepx=='%'?"%":"",
                    y,sepy=='%'?"%":"",
                    z,
                    gmic_selection.data(),
                    opacity,
                    _render3d==0?"dot":_render3d==1?"wireframe":_render3d==2?"flat":
                    _render3d==3?"flat-shaded":_render3d==4?"gouraud-shaded":"phong-shaded",
                    _is_double3d?"double":"simple",
                    is_zbuffer?"":"no ",
                    _focale3d,_light3d_x,_light3d_y,_light3d_z,
                    _specular_lightness3d,_specular_shininess3d);
              CImgList<float> opacities;
              vertices.assign(img0,false);
              try {
                if (_render3d>=3) {
                  vertices.CImg3dtoobject3d(primitives,g_list_uc,opacities,false);
                  if (light3d) g_list_uc.insert(light3d,~0U,true);
                } else vertices.CImg3dtoobject3d(primitives,g_list_f,opacities,false);
              } catch (CImgException&) {
                if (!vertices.is_CImg3d(true,&(*message=0)))
                  error(images,0,0,
                        "Command '-object3d': Invalid 3d object [%u], specified "
                        "in argument '%s' (%s).",
                        *ind,gmic_argument_text(),message.data());
                else throw;
              }

              cimglist_for(opacities,o) if (!opacities[o].is_shared()) opacities[o]*=opacity;
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                const float
                  nx = sepx=='%'?x*(img.width() - 1)/100:x,
                  ny = sepy=='%'?y*(img.height() - 1)/100:y;
                CImg<float> zbuffer(is_zbuffer?img.width():0,is_zbuffer?img.height():0,1,1,0);
                if (g_list_f) {
                  gmic_apply(draw_object3d(nx,ny,z,vertices,primitives,g_list_f,opacities,
                                           _render3d,_is_double3d,_focale3d,
                                           _light3d_x,_light3d_y,_light3d_z,
                                           _specular_lightness3d,_specular_shininess3d,
                                           zbuffer));
                  g_list_f.assign();
                } else {
                  gmic_apply(draw_object3d(nx,ny,z,vertices,primitives,g_list_uc,opacities,
                                           _render3d,_is_double3d,_focale3d,
                                           _light3d_x,_light3d_y,_light3d_z,
                                           _specular_lightness3d,_specular_shininess3d,
                                           zbuffer));
                  g_list_uc.assign();
                }
              }
            } else arg_error("object3d");
            vertices.assign();
            primitives.assign();
            is_released = false; ++position; continue;
          }

          // Bitwise or.
          gmic_arithmetic_command("-or",
                                  operator|=,
                                  "Compute bitwise OR of image%s by %g%s",
                                  gmic_selection.data(),value,ssep,Tlong,
                                  operator|=,
                                  "Compute bitwise OR of image%s by image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute bitwise OR of image%s by expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute sequential bitwise OR of image%s");

          // Set 3d object opacity.
          if (!std::strcmp("-opacity3d",command)) {
            gmic_substitute_args();
            value = 1;
            if (cimg_sscanf(argument,"%lf%c",
                            &value,&end)==1) ++position;
            else value = 1;
            print(images,0,"Set opacity of 3d object%s to %g.",
                  gmic_selection.data(),
                  value);
            cimg_forY(selection,l) {
              const unsigned int uind = selection[l];
              CImg<T>& img = images[uind];
              try { gmic_apply(color_CImg3d(0,0,0,(float)value,false,true)); }
              catch (CImgException&) {
                if (!img.is_CImg3d(true,&(*message=0)))
                  error(images,0,0,
                        "Command '-opacity3d': Invalid 3d object [%d], "
                        "in selected image%s (%s).",
                        uind,gmic_selection.data(),message.data());
                else throw;
              }
            }
            is_released = false; continue;
          }

          // Output.
          if (!is_double_hyphen && !std::strcmp("-output",command)) {
            gmic_substitute_args();

            // Set good alias for shared variables.
            CImg<char> &_filename = _color, &filename_tmp = _title, &options = _argc;
            char cext[12];
            *cext = *_filename = *filename_tmp = *options = 0;
            CImgList<unsigned int> empty_indices;
            CImg<char> eselec;

            if (cimg_sscanf(argument,"%11[a-zA-Z]:%4095[^,],%255s",  // Detect forced file format.
                            cext,_filename.data(),options.data())<2 ||
                !cext[1]) {  // length of preprend 'ext' must be >=2 (avoid case 'C:\\..' on Windows).
              *cext = *_filename = *options = 0;
              if (cimg_sscanf(argument,"%4095[^,],%255s",_filename.data(),options.data())!=2) {
                std::strncpy(_filename,argument,_filename.width() - 1);
                _filename[_filename.width() - 1] = 0;
              }
            }
            strreplace_fw(_filename);
            strreplace_fw(options);

            if (*cext) { // Force output to be written as a '.ext' file : generate random filename.
              if (*_filename=='-' && (!_filename[1] || _filename[1]=='.')) {
                // Simplify filename 'ext:-.foo' as '-.ext'.
                cimg_snprintf(_filename,_filename.width(),"-.%s",cext);
                *cext = 0;
              } else {
                std::FILE *file = 0;
                do {
                  cimg_snprintf(filename_tmp,filename_tmp.width(),"%s%c%s.%s",
                                cimg::temporary_path(),cimg_file_separator,
                                cimg::filenamerand(),cext);
                  if ((file=std::fopen(filename_tmp,"rb"))!=0) std::fclose(file);
                } while (file);
              }
            }
            const char
              *const filename = *cext?filename_tmp:_filename,
              *const ext = cimg::split_filename(filename);
            CImg<char> uext = CImg<char>::string(ext);
            cimg::uncase(uext);

            if (!cimg::strcasecmp(ext,"off")) {
              *formula = 0;
              std::strncpy(formula,filename,_formula.width() - 1);
              formula[_formula.width() - 1] = 0;

              if (*options)
                error(images,0,0,
                      "Command '-output': File '%s', format does not take any output options (options '%s' specified).",
                      formula,options.data());

              print(images,0,"Output 3d object%s as %s file '%s'.",
                    gmic_selection.data(),uext.data(),formula);

              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                const CImg<T>& img = gmic_check(images[uind]);
                if (selection.height()!=1) cimg::number_filename(filename,l,6,formula);
                CImgList<float> opacities;
                vertices.assign(img,false);
                try {
                  vertices.CImg3dtoobject3d(primitives,g_list_f,opacities,false).
                    save_off(primitives,g_list_f,formula);
                } catch (CImgException&) {
                  if (!vertices.is_CImg3d(true,&(*message=0)))
                    error(images,0,0,
                          "Command '-output': 3d object file '%s', invalid 3d object [%u] "
                          "in selected image%s (%s).",
                          formula,uind,gmic_selection.data(),message.data());
                  else throw;
                }
              }
              vertices.assign();
              primitives.assign();
              g_list_f.assign();
            } else if (!cimg::strcasecmp(ext,"cpp") || !cimg::strcasecmp(ext,"c") ||
                       !cimg::strcasecmp(ext,"hpp") || !cimg::strcasecmp(ext,"h") ||
                       !cimg::strcasecmp(ext,"pan")) {
              const char *const
                stype = (cimg_sscanf(options,"%255[a-zA-Z]%c",&(*argx=0),&(end=0))==1 ||
                         (cimg_sscanf(options,"%255[a-zA-Z]%c",&(*argx=0),&end)==2 && end==','))?
                argx:cimg::type<T>::string();
              g_list.assign(selection.height());
              cimg_forY(selection,l) if (!gmic_check(images[selection(l)]))
                CImg<unsigned int>::vector(selection(l)).move_to(empty_indices);
              if (empty_indices && is_verbose) {
                selection2string(empty_indices>'y',images_names,1,eselec);
                warn(images,0,false,
                     "Command '-output': Image%s %s empty.",
                     eselec.data(),empty_indices.size()>1?"are":"is");
              }
              cimg_forY(selection,l)
                g_list[l].assign(images[selection[l]],images[selection[l]]?true:false);
              if (g_list.size()==1)
                print(images,0,
                      "Output image%s as %s file '%s', with pixel type '%s' (1 image %dx%dx%dx%d).",
                      gmic_selection.data(),
                      uext.data(),_filename.data(),
                      stype,
                      g_list[0].width(),g_list[0].height(),
                      g_list[0].depth(),g_list[0].spectrum());
              else print(images,0,"Output image%s as %s file '%s', with pixel type '%s'.",
                         gmic_selection.data(),
                         uext.data(),_filename.data(),
                         stype);
              if (!g_list)
                error(images,0,0,
                      "Command '-output': File '%s', instance list (%u,%p) is empty.",
                      _filename.data(),g_list.size(),g_list.data());

#define gmic_save_multitype(value_type,svalue_type) \
              if (!std::strcmp(stype,svalue_type)) { \
                if (g_list.size()==1) \
                  CImg<value_type>(g_list[0], \
                                   cimg::type<T>::string()==cimg::type<value_type>::string()). \
                    save(filename); \
                else { \
                  cimglist_for(g_list,l) { \
                    cimg::number_filename(filename,l,6,formula); \
                    CImg<value_type>(g_list[l], \
                                     cimg::type<T>::string()==cimg::type<value_type>::string()). \
                                     save(formula); \
                  } \
                } \
              }
              gmic_save_multitype(unsigned char,"uchar")
              else gmic_save_multitype(unsigned char,"unsigned char")
                else gmic_save_multitype(char,"char")
                  else gmic_save_multitype(unsigned short,"ushort")
                    else gmic_save_multitype(unsigned short,"unsigned short")
                      else gmic_save_multitype(short,"short")
                        else gmic_save_multitype(unsigned int,"uint")
                          else gmic_save_multitype(unsigned int,"unsigned int")
                            else gmic_save_multitype(int,"int")
                              else gmic_save_multitype(unsigned long,"ulong")
                                else gmic_save_multitype(unsigned long,"unsigned long")
                                  else gmic_save_multitype(long,"long")
                                    else gmic_save_multitype(float,"float")
                                      else gmic_save_multitype(double,"double")
                                        else error(images,0,0,
                                                   "Command '-output': File '%s', invalid "
                                                   "specified pixel type '%s'.",
                                                   _filename.data(),stype);
            } else if (!cimg::strcasecmp(ext,"tiff") || !cimg::strcasecmp(ext,"tif")) {
              const char *const
                stype = (cimg_sscanf(options,"%255[a-zA-Z]%c",&(*argx=0),&(end=0))==1 ||
                         (cimg_sscanf(options,"%255[a-zA-Z]%c",&(*argx=0),&end)==2 && end==','))?
                argx:cimg::type<T>::string();
              const unsigned int l_stype = (unsigned int)std::strlen(stype);
              const char *const _options = options.data() + (stype!=argx?0:l_stype + (end==','?1:0));
              float _is_multipage = 0;
              if (cimg_sscanf(_options,"%255[a-zA-Z]%c",argy,&end)!=1 &&
                  cimg_sscanf(_options,"%255[a-zA-Z],%f%c",argy,&_is_multipage,&end)!=2) {
                *argy = 0; _is_multipage = 0;
              }
              const unsigned int compression_type = !cimg::strcasecmp(argy,"jpeg") || !cimg::strcasecmp(argy,"jpg")?2:
                !cimg::strcasecmp(argy,"lzw")?1U:0U;
              const bool is_multipage = (bool)cimg::round(_is_multipage);

              g_list.assign(selection.height());
              cimg_forY(selection,l) if (!gmic_check(images[selection(l)]))
                CImg<unsigned int>::vector(selection(l)).move_to(empty_indices);
              if (empty_indices && is_verbose) {
                selection2string(empty_indices>'y',images_names,1,eselec);
                warn(images,0,false,
                     "Command '-output': Image%s %s empty.",
                     eselec.data(),empty_indices.size()>1?"are":"is");
              }
              cimg_forY(selection,l)
                g_list[l].assign(images[selection[l]],g_list[l]?true:false);
              if (g_list.size()==1)
                print(images,0,"Output image%s as %s file '%s', with pixel type '%s' and %s compression "
                      "(1 image %dx%dx%dx%d).",
                      gmic_selection.data(),
                      uext.data(),_filename.data(),stype,
                      compression_type==2?"JPEG":compression_type==1?"LZW":"no",
                      g_list[0].width(),g_list[0].height(),
                      g_list[0].depth(),g_list[0].spectrum());
              else print(images,0,"Output image%s as %s file '%s', with pixel type '%s', "
                         "%s compression and %s-page mode.",
                         gmic_selection.data(),
                         uext.data(),_filename.data(),stype,
                         compression_type==2?"JPEG":compression_type==1?"LZW":"no",
                         is_multipage?"multi":"single");
              if (!g_list)
                error(images,0,0,
                      "Command '-output': File '%s', instance list (%u,%p) is empty.",
                      _filename.data(),g_list.size(),g_list.data());

#define gmic_save_tiff(value_type,svalue_type) \
              if (!std::strcmp(stype,svalue_type)) { \
                if (g_list.size()==1 || is_multipage) \
                  CImgList<value_type>(g_list, \
                                   cimg::type<T>::string()==cimg::type<value_type>::string()). \
                    save_tiff(filename,compression_type); \
                else { \
                  cimglist_for(g_list,l) { \
                    cimg::number_filename(filename,l,6,formula); \
                    CImg<value_type>(g_list[l], \
                                   cimg::type<T>::string()==cimg::type<value_type>::string()). \
                      save_tiff(formula,compression_type); \
                  } \
                } \
              }
              gmic_save_tiff(unsigned char,"uchar")
              else gmic_save_tiff(unsigned char,"unsigned char")
                else gmic_save_tiff(char,"char")
                  else gmic_save_tiff(unsigned short,"ushort")
                    else gmic_save_tiff(unsigned short,"unsigned short")
                      else gmic_save_tiff(short,"short")
                        else gmic_save_tiff(unsigned int,"uint")
                          else gmic_save_tiff(unsigned int,"unsigned int")
                            else gmic_save_tiff(int,"int")
                              else gmic_save_tiff(unsigned long,"ulong")
                                else gmic_save_tiff(unsigned long,"unsigned long")
                                  else gmic_save_tiff(long,"long")
                                    else gmic_save_tiff(float,"float")
                                      else gmic_save_tiff(double,"double")
                                        else error(images,0,0,
                                                   "Command '-output': File '%s', invalid "
                                                   "specified pixel type '%s'.",
                                                   _filename.data(),stype);

            } else if (!cimg::strcasecmp(ext,"gif")) {
              float fps = 0, _nb_loops = 0;
              g_list.assign(selection.height());
              cimg_forY(selection,l) if (!gmic_check(images[selection(l)]))
                CImg<unsigned int>::vector(selection(l)).move_to(empty_indices);
              if (empty_indices && is_verbose) {
                selection2string(empty_indices>'y',images_names,1,eselec);
                warn(images,0,false,
                     "Command '-output': Image%s %s empty.",
                     eselec.data(),empty_indices.size()>1?"are":"is");
              }
              cimg_forY(selection,l)
                g_list[l].assign(images[selection[l]],g_list[l]?true:false);
              if (g_list.size()>1 && cimg_sscanf(options,"%f,%f",&fps,&_nb_loops)>=1 && fps>0) {
                // Save animated .gif file.
                const unsigned int nb_loops = (unsigned int)cimg::round(_nb_loops);
                if (nb_loops)
                  print(images,0,
                        "Output image%s as animated %s file '%s', with %g fps and %u loops.",
                        gmic_selection.data(),uext.data(),_filename.data(),fps,nb_loops);
                else
                  print(images,0,
                        "Output image%s as animated %s file '%s', with %g fps.",
                        gmic_selection.data(),uext.data(),_filename.data(),fps);
                g_list.save_gif_external(filename,fps,nb_loops);
              } else {
                if (g_list.size()==1)
                  print(images,0,"Output image%s as %s file '%s' (1 image %dx%dx%dx%d).",
                        gmic_selection.data(),
                        uext.data(),_filename.data(),
                        g_list[0].width(),g_list[0].height(),
                        g_list[0].depth(),g_list[0].spectrum());
                else print(images,0,"Output image%s as %s file '%s'.",
                           gmic_selection.data(),uext.data(),_filename.data());
                g_list.save(filename); // Save distinct .gif files.
              }
            } else if (!cimg::strcasecmp(ext,"jpeg") || !cimg::strcasecmp(ext,"jpg")) {
              float quality = 100;
              if (cimg_sscanf(options,"%f%c",&quality,&end)!=1) quality = 100;
              if (quality<0) quality = 0; else if (quality>100) quality = 100;
              g_list.assign(selection.height());
              cimg_forY(selection,l) if (!gmic_check(images[selection(l)]))
                CImg<unsigned int>::vector(selection(l)).move_to(empty_indices);
              if (empty_indices && is_verbose) {
                selection2string(empty_indices>'y',images_names,1,eselec);
                warn(images,0,false,
                     "Command '-output': Image%s %s empty.",
                     eselec.data(),empty_indices.size()>1?"are":"is");
              }
              cimg_forY(selection,l)
                g_list[l].assign(images[selection[l]],g_list[l]?true:false);
              if (g_list.size()==1)
                print(images,0,
                      "Output image%s as %s file '%s', with quality %g%% (1 image %dx%dx%dx%d).",
                      gmic_selection.data(),
                      uext.data(),_filename.data(),
                      quality,
                      g_list[0].width(),g_list[0].height(),
                      g_list[0].depth(),g_list[0].spectrum());
              else print(images,0,"Output image%s as %s file '%s', with quality %g%%.",
                         gmic_selection.data(),
                         uext.data(),_filename.data(),
                         quality);
              if (!g_list)
                error(images,0,0,
                      "Command '-output': File '%s', instance list (%u,%p) is empty.",
                      _filename.data(),g_list.size(),g_list.data());
              if (g_list.size()==1)
                g_list[0].save_jpeg(filename,(unsigned int)cimg::round(quality));
              else {
                cimglist_for(g_list,l) {
                  cimg::number_filename(filename,l,6,formula);
                  g_list[l].save_jpeg(formula,(unsigned int)cimg::round(quality));
                }
              }
            } else if (!cimg::strcasecmp(ext,"mnc") && *options) {
              g_list.assign(selection.height());
              cimg_forY(selection,l) if (!gmic_check(images[selection(l)]))
                CImg<unsigned int>::vector(selection(l)).move_to(empty_indices);
              if (empty_indices && is_verbose) {
                selection2string(empty_indices>'y',images_names,1,eselec);
                warn(images,0,false,
                     "Command '-output': Image%s %s empty.",
                     eselec.data(),empty_indices.size()>1?"are":"is");
              }
              cimg_forY(selection,l)
                g_list[l].assign(images[selection[l]],g_list[l]?true:false);
              if (g_list.size()==1)
                print(images,0,
                      "Output image%s as %s file '%s', with header get from file '%s' "
                      "(1 image %dx%dx%dx%d).",
                      gmic_selection.data(),
                      uext.data(),_filename.data(),
                      options.data(),
                      g_list[0].width(),g_list[0].height(),
                      g_list[0].depth(),g_list[0].spectrum());
              else
                print(images,0,
                      "Output image%s as %s file '%s', with header get from file '%s'.",
                      gmic_selection.data(),
                      uext.data(),_filename.data(),
                      options.data());
              if (g_list.size()==1)
                g_list[0].save_minc2(filename,options);
              else {
                cimglist_for(g_list,l) {
                  cimg::number_filename(filename,l,6,formula);
                  g_list[l].save_minc2(formula,options);
                }
              }
            } else if (!cimg::strcasecmp(ext,"raw")) {
              const char *const stype = cimg_sscanf(options,"%255[a-zA-Z]%c",argx,&end)==1?argx:
                cimg::type<T>::string();
              g_list.assign(selection.height());
              cimg_forY(selection,l) if (!gmic_check(images[selection(l)]))
                CImg<unsigned int>::vector(selection(l)).move_to(empty_indices);
              if (empty_indices && is_verbose) {
                selection2string(empty_indices>'y',images_names,1,eselec);
                warn(images,0,false,
                     "Command '-output': Image%s %s empty.",
                     eselec.data(),empty_indices.size()>1?"are":"is");
              }
              cimg_forY(selection,l)
                g_list[l].assign(images[selection[l]],images[selection[l]]?true:false);
              if (g_list.size()==1)
                print(images,0,
                      "Output image%s as %s file '%s', with pixel type '%s' (1 image %dx%dx%dx%d).",
                      gmic_selection.data(),
                      uext.data(),_filename.data(),
                      stype,
                      g_list[0].width(),g_list[0].height(),
                      g_list[0].depth(),g_list[0].spectrum());
              else print(images,0,"Output image%s as %s file '%s', with pixel type '%s'.",
                         gmic_selection.data(),
                         uext.data(),_filename.data(),
                         stype);
              if (!g_list)
                error(images,0,0,
                      "Command '-output': File '%s', instance list (%u,%p) is empty.",
                      _filename.data(),g_list.size(),g_list.data());

#define gmic_save_raw(value_type,svalue_type) \
              if (!std::strcmp(stype,svalue_type)) { \
                if (g_list.size()==1) \
                  CImg<value_type>(g_list[0], \
                                   cimg::type<T>::string()==cimg::type<value_type>::string()). \
                    save_raw(filename); \
                else { \
                  cimglist_for(g_list,l) { \
                    cimg::number_filename(filename,l,6,formula); \
                    CImg<value_type>(g_list[l], \
                                     cimg::type<T>::string()==cimg::type<value_type>::string()). \
                                     save_raw(formula); \
                  } \
                } \
              }
              gmic_save_raw(unsigned char,"uchar")
              else gmic_save_raw(unsigned char,"unsigned char")
                else gmic_save_raw(char,"char")
                  else gmic_save_raw(unsigned short,"ushort")
                    else gmic_save_raw(unsigned short,"unsigned short")
                      else gmic_save_raw(short,"short")
                        else gmic_save_raw(unsigned int,"uint")
                          else gmic_save_raw(unsigned int,"unsigned int")
                            else gmic_save_raw(int,"int")
                              else gmic_save_raw(unsigned long,"ulong")
                                else gmic_save_raw(unsigned long,"unsigned long")
                                  else gmic_save_raw(long,"long")
                                    else gmic_save_raw(float,"float")
                                      else gmic_save_raw(double,"double")
                                        else error(images,0,0,
                                                   "Command '-output': File '%s', invalid "
                                                   "specified pixel type '%s'.",
                                                   _filename.data(),stype);
            } else if (!cimg::strcasecmp(ext,"cimg") || !cimg::strcasecmp(ext,"cimgz")) {
              const char *const stype = cimg_sscanf(options,"%255[a-zA-Z]%c",argx,&end)==1?argx:
                cimg::type<T>::string();
              g_list.assign(selection.height());
              cimg_forY(selection,l)
                g_list[l].assign(images[selection[l]],images[selection[l]]?true:false);
              print(images,0,"Output image%s as %s file '%s', with pixel type '%s'.",
                    gmic_selection.data(),
                    uext.data(),_filename.data(),
                    stype);

#define gmic_save_cimg(value_type,svalue_type) \
              if (!std::strcmp(stype,svalue_type)) \
                CImgList<value_type>(g_list, \
                                     cimg::type<T>::string()==cimg::type<value_type>::string()). \
                                     save(filename);
              gmic_save_cimg(unsigned char,"uchar")
              else gmic_save_cimg(unsigned char,"unsigned char")
                else gmic_save_cimg(char,"char")
                  else gmic_save_cimg(unsigned short,"ushort")
                    else gmic_save_cimg(unsigned short,"unsigned short")
                      else gmic_save_cimg(short,"short")
                        else gmic_save_cimg(unsigned int,"uint")
                          else gmic_save_cimg(unsigned int,"unsigned int")
                            else gmic_save_cimg(int,"int")
                              else gmic_save_cimg(unsigned long,"ulong")
                                else gmic_save_cimg(unsigned long,"unsigned long")
                                  else gmic_save_cimg(long,"long")
                                    else gmic_save_cimg(float,"float")
                                      else gmic_save_cimg(double,"double")
                                        else error(images,0,0,
                                                   "Command '-output': File '%s', invalid "
                                                   "specified pixel type '%s'.",
                                                   _filename.data(),stype);
            } else if (!cimg::strcasecmp(ext,"gmz") || !*ext) {
              const char *const stype = cimg_sscanf(options,"%255[a-zA-Z]%c",argx,&end)==1?argx:
                cimg::type<T>::string();
              g_list.assign(selection.height());
              CImgList<char> gmz_info(1 + selection.height());
              CImg<char>::string("GMZ").move_to(gmz_info[0]);
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                g_list[l].assign(images[uind],images[uind]?true:false);
                CImg<char>::string(images_names[uind]).move_to(gmz_info[1 + l]);
              }
              (gmz_info>'x').unroll('y').move_to(g_list);
              print(images,0,"Output image%s as %s file '%s', with pixel type '%s'.",
                    gmic_selection.data(),
                    uext.data(),_filename.data(),
                    stype);

#define gmic_save_gmz(value_type,svalue_type) \
              if (!std::strcmp(stype,svalue_type)) \
                CImgList<value_type>(g_list, \
                                     cimg::type<T>::string()==cimg::type<value_type>::string()). \
                                     save_cimg(filename,true);
              gmic_save_gmz(unsigned char,"uchar")
              else gmic_save_gmz(unsigned char,"unsigned char")
                else gmic_save_gmz(char,"char")
                  else gmic_save_gmz(unsigned short,"ushort")
                    else gmic_save_gmz(unsigned short,"unsigned short")
                      else gmic_save_gmz(short,"short")
                        else gmic_save_gmz(unsigned int,"uint")
                          else gmic_save_gmz(unsigned int,"unsigned int")
                            else gmic_save_gmz(int,"int")
                              else gmic_save_gmz(unsigned long,"ulong")
                                else gmic_save_gmz(unsigned long,"unsigned long")
                                  else gmic_save_gmz(long,"long")
                                    else gmic_save_gmz(float,"float")
                                      else gmic_save_gmz(double,"double")
                                        else error(images,0,0,
                                                   "Command '-output': File '%s', invalid "
                                                   "specified pixel type '%s'.",
                                                   _filename.data(),stype);
            } else if (!cimg::strcasecmp(ext,"avi") ||
                       !cimg::strcasecmp(ext,"mov") ||
                       !cimg::strcasecmp(ext,"asf") ||
                       !cimg::strcasecmp(ext,"divx") ||
                       !cimg::strcasecmp(ext,"flv") ||
                       !cimg::strcasecmp(ext,"mpg") ||
                       !cimg::strcasecmp(ext,"m1v") ||
                       !cimg::strcasecmp(ext,"m2v") ||
                       !cimg::strcasecmp(ext,"m4v") ||
                       !cimg::strcasecmp(ext,"mjp") ||
                       !cimg::strcasecmp(ext,"mp4") ||
                       !cimg::strcasecmp(ext,"mkv") ||
                       !cimg::strcasecmp(ext,"mpe") ||
                       !cimg::strcasecmp(ext,"movie") ||
                       !cimg::strcasecmp(ext,"ogm") ||
                       !cimg::strcasecmp(ext,"ogg") ||
                       !cimg::strcasecmp(ext,"ogv") ||
                       !cimg::strcasecmp(ext,"qt") ||
                       !cimg::strcasecmp(ext,"rm") ||
                       !cimg::strcasecmp(ext,"vob") ||
                       !cimg::strcasecmp(ext,"wmv") ||
                       !cimg::strcasecmp(ext,"xvid") ||
                       !cimg::strcasecmp(ext,"mpeg")) {
              float fps = 0, keep_open = 0;
              name.assign(8); *name = 0; // codec
              cimg_sscanf(options,"%f,%7[a-zA-Z0-9],%f",&fps,name.data(),&keep_open);
              fps = cimg::round(fps);
              if (!fps) fps = 25;
              if (*name=='0' && !name[1]) *name = 0;
              g_list.assign(selection.height());
              cimg_forY(selection,l) if (!gmic_check(images[selection(l)]))
                CImg<unsigned int>::vector(selection(l)).move_to(empty_indices);
              if (empty_indices && is_verbose) {
                selection2string(empty_indices>'y',images_names,1,eselec);
                warn(images,0,false,
                     "Command '-output': Image%s %s empty.",
                     eselec.data(),empty_indices.size()>1?"are":"is");
              }
              cimg_forY(selection,l)
                g_list[l].assign(images[selection[l]],g_list[l]?true:false);
              print(images,0,"Output image%s as %s file '%s', with %g fps and %s codec.",
                    gmic_selection.data(),
                    uext.data(),_filename.data(),
                    fps,*name?name.data():"(default)");
              try {
                g_list.save_video(filename,(unsigned int)fps,name,(bool)keep_open);
              } catch (CImgException&) {
                warn(images,0,false,
                     "Command '-output': Cannot encode file '%s' natively. Trying fallback function.",
                     filename);
                g_list.save_ffmpeg_external(filename,(unsigned int)fps);
              }
            } else {
              g_list.assign(selection.height());
              cimg_forY(selection,l) if (!gmic_check(images[selection(l)]))
                CImg<unsigned int>::vector(selection(l)).move_to(empty_indices);
              if (empty_indices && is_verbose) {
                selection2string(empty_indices>'y',images_names,1,eselec);
                warn(images,0,false,
                     "Command '-output': Image%s %s empty.",
                     eselec.data(),empty_indices.size()>1?"are":"is");
              }
              cimg_forY(selection,l)
                g_list[l].assign(images[selection[l]],g_list[l]?true:false);
              if (g_list.size()==1)
                print(images,0,"Output image%s as %s file '%s' (1 image %dx%dx%dx%d).",
                      gmic_selection.data(),
                      uext.data(),_filename.data(),
                      g_list[0].width(),g_list[0].height(),
                      g_list[0].depth(),g_list[0].spectrum());
              else print(images,0,"Output image%s as %s file '%s'.",
                         gmic_selection.data(),uext.data(),_filename.data());

              if (*options)
                error(images,0,0,
                      "Command '-output': File '%s', format '%s' does not take any output options "
                      "(options '%s' specified).",
                      _filename.data(),ext,options.data());

              g_list.save(filename);
            }

            if (*cext) { // When output forced to 'ext' : copy final file to specified location.
              try {
                CImg<unsigned char>::get_load_raw(filename_tmp).save_raw(_filename);
                std::remove(filename_tmp);
              } catch (...) { // Failed, maybe 'filename_tmp' consists of several numbered images.
                bool save_failure = false;
                *message = 0;
                for (unsigned int i = 0; i!=~0U; ++i) {
                  cimg::number_filename(filename_tmp,i,6,formula);
                  cimg::number_filename(_filename,i,6,message);
                  try { CImg<unsigned char>::get_load_raw(formula).save_raw(message); }
                  catch (...) { i = ~0U - 1; if (!i) save_failure = true; }
                }
                if (save_failure)
                  error(images,0,0,
                        "Command '-output': Invalid write of file '%s' from temporary file '%s'.",
                        _filename.data(),filename_tmp.data());
              }
            }
            g_list.assign();
            is_released = true; ++position; continue;
          }

        } // command1=='o'.

        //----------------------------
        // Commands starting by '-p..'
        //----------------------------
        else if (command1=='p') {

          // Pass image from parent context.
          if (!is_double_hyphen && !std::strcmp("-pass",command)) {
            gmic_substitute_args();
            unsigned int shared_state = 2;
            if (cimg_sscanf(argument,"%u%c",&shared_state,&end)==1 && shared_state<=2) ++position;
            else shared_state = 2;
            print(images,0,"Insert image%s from parent context %s%s.",
                  gmic_selection.data(),
                  shared_state==0?"in non-shared state":
                  shared_state==1?"in shared state":"using adaptive state",
                  selection.height()>1?"s":"");

            cimg_forY(selection,l) {
              CImg<T> &img = parent_images[selection[l]];
              const T *p = 0;
              std::memcpy(&p,&img._width,sizeof(void*));

              if (p && !img.data()) {
                // Parent image is in the current selection -> must search the current list.
                bool found_image = false;
                cimglist_for(images,i) {
                  if (images[i].data()==p) { // Found it !
                    images.insert(images[i],~0U,shared_state==1);
                    images_names.insert(images_names[i].get_copymark());
                    found_image = true;
                    break;
                  }
                }
                if (!found_image) error(images,0,0,
                                        "Command '-pass': Unreferenced image [%d] from parent context "
                                        "(has been re-allocated in current context or reserved by another thread).",
                                        selection[l]);
              } else { // Easy case, parent image not in the current selection.
                images.insert(img,~0U,(bool)shared_state);
                images_names.insert(parent_images_names[selection[l]].get_copymark());
              }
            }
            is_released = false; continue;
          }

          // Run multiple commands in parallel.
          if (!std::strcmp("-parallel",item)) {
            gmic_substitute_args();
            const char *_arg = argument, *_arg_text = gmic_argument_text_printed();
            bool wait_mode = true;
            if ((*_arg=='0' || *_arg=='1') && (_arg[1]==',' || !_arg[1])) {
              wait_mode = (bool)(*_arg - '0'); _arg+=2; _arg_text+=2;
            }
            CImgList<char> arguments = CImg<char>::string(_arg).get_split(CImg<char>::vector(','),0,false);
            CImg<st_gmic_parallel<T> >(1,arguments.width()).move_to(threads_data);
            CImg<st_gmic_parallel<T> > &_threads_data = threads_data.back();

#ifdef gmic_is_parallel
            print(images,0,"Execute %d command%s '%s' in parallel%s.",
                  arguments.width(),arguments.width()>1?"s":"",_arg_text,
                  wait_mode?" and wait thread termination immediately":
                  " and wait thread termination when current environment ends");
#else // #ifdef gmic_is_parallel
            print(images,0,"Execute %d commands '%s' (run sequentially, "
                  "parallel computing disabled).",
                  arguments.width(),_arg_text);
#endif // #ifdef gmic_is_parallel

            // Prepare thread structures.
            cimg_forY(_threads_data,l) {
              gmic &gi = _threads_data[l].gmic_instance;
              for (unsigned int i = 0; i<512; ++i) {
                gi.commands[i].assign(commands[i],true);
                gi.commands_names[i].assign(commands_names[i],true);
                gi.commands_has_arguments[i].assign(commands_has_arguments[i],true);

                if (i==511) { // Share inter-thread global variables.
                  gi.variables[i] = variables[i];
                  gi.variables_names[i] = variables_names[i];
                } else {
                  if (i==510) { // Make a copy of single-thread global variables.
                    gi._variables[i].assign(_variables[i]);
                    gi._variables_names[i].assign(_variables_names[i]);
                    _threads_data[l].variables_sizes[i] = variables_sizes[i];
                  } else _threads_data[l].variables_sizes[i] = 0;
                  gi.variables[i] = &gi._variables[i];
                  gi.variables_names[i] = &gi._variables_names[i];
                }
              }

              gi.callstack.assign(callstack);
              gi.commands_files.assign(commands_files,true);
              cimg_snprintf(title,_title.width(),"*thread%d",l);
              CImg<char>::string(title).move_to(gi.callstack);
              gi.light3d.assign(light3d);
              gi.status.assign(status);
              gi.debug_filename = debug_filename;
              gi.debug_line = debug_line;
              gi.focale3d = focale3d;
              gi.light3d_x = light3d_x;
              gi.light3d_y = light3d_y;
              gi.light3d_z = light3d_z;
              gi.specular_lightness3d = specular_lightness3d;
              gi.specular_shininess3d = specular_shininess3d;
              gi._progress = 0;
              gi.progress = &gi._progress;
              gi.is_released = is_released;
              gi.is_debug = is_debug;
              gi.is_start = false;
              gi.is_quit = false;
              gi.is_return = false;
              gi.is_double3d = is_double3d;
              gi.check_elif = false;
              gi.verbosity = verbosity;
              gi.render3d = render3d;
              gi.renderd3d = renderd3d;
              gi._is_abort = _is_abort;
              gi.is_abort = is_abort;
              gi.is_abort_thread = false;
              gi.nb_carriages = nb_carriages;
              gi.reference_time = reference_time;
              _threads_data[l].images = &images;
              _threads_data[l].images_names = &images_names;
              _threads_data[l].parent_images = &parent_images;
              _threads_data[l].parent_images_names = &parent_images_names;
              _threads_data[l].threads_data = &threads_data;
              _threads_data[l].command_selection = command_selection;
              _threads_data[l].is_thread_running = true;

              // Substitute special characters codes appearing outside strings.
              arguments[l].resize(1,arguments[l].height() + 1,1,1,0);
              bool is_dquoted = false;
              for (char *s = arguments[l].data(); *s; ++s) {
                const char c = *s;
                if (c=='\"') is_dquoted = !is_dquoted;
                if (!is_dquoted) *s = c<' '?(c==gmic_dollar?'$':c==gmic_lbrace?'{':c==gmic_rbrace?'}':
                                             c==gmic_comma?',':c==gmic_dquote?'\"':c):c;
              }
              gi.commands_line_to_CImgList(arguments[l].data()).
                move_to(_threads_data[l].commands_line);
            }

            // Run threads.
            cimg_forY(_threads_data,l) {
#ifdef gmic_is_parallel
#if cimg_OS!=2

#if defined(__MACOSX__) || defined(__APPLE__)
              const unsigned long stacksize = 8*1024*1024;
              pthread_attr_t thread_attr;
              if (!pthread_attr_init(&thread_attr) && !pthread_attr_setstacksize(&thread_attr,stacksize))
                // Reserve enough stack size for the new thread.
                pthread_create(&_threads_data[l].thread_id,&thread_attr,gmic_parallel<T>,(void*)&_threads_data[l]);
              else
#endif // #if defined(__MACOSX__) || defined(__APPLE__)
                pthread_create(&_threads_data[l].thread_id,0,gmic_parallel<T>,(void*)&_threads_data[l]);

#else // #if cimg_OS!=2
              _threads_data[l].thread_id = CreateThread(0,0,gmic_parallel<T>,
                                                        (void*)&_threads_data[l],0,0);
#endif // #if cimg_OS!=2
#else // #ifdef gmic_is_parallel
              gmic_parallel<T>((void*)&_threads_data[l]);
#endif // #ifdef gmic_is_parallel
            }

            // Wait threads if immediate waiting mode selected.
            if (wait_mode) {
              cimg_forY(_threads_data,l) if (_threads_data[l].is_thread_running) {
#ifdef gmic_is_parallel
#if cimg_OS!=2
                pthread_join(_threads_data[l].thread_id,0);
#else // #if cimg_OS!=2
                WaitForSingleObject(_threads_data[l].thread_id,INFINITE);
                CloseHandle(_threads_data[l].thread_id);
#endif // #if cimg_OS!=2
                _threads_data[l].is_thread_running = false;
#endif // #ifdef gmic_is_parallel
              }

              // Get 'released' state of the image list.
              cimg_forY(_threads_data,l) is_released&=_threads_data[l].gmic_instance.is_released;

              // Get status modified by first thread.
              _threads_data[0].gmic_instance.status.move_to(status);

              // Check for possible exceptions thrown by threads.
              cimg_forY(_threads_data,l) if (_threads_data[l].exception._message)
                throw _threads_data[l].exception;

              threads_data.remove();
            }

            ++position; continue;
          }

          // Permute axes.
          if (!std::strcmp("-permute",command)) {
            gmic_substitute_args();
            print(images,0,"Permute axes of image%s, with permutation '%s'.",
                  gmic_selection.data(),gmic_argument_text_printed());
            cimg_forY(selection,l) gmic_apply(permute_axes(argument));
            is_released = false; ++position; continue;
          }

          // Set progress indice.
          if (!std::strcmp("-progress",item)) {
            gmic_substitute_args();
            value = -1;
            if (cimg_sscanf(argument,"%lf%c",
                            &value,&end)==1) {
              if (value<0) value = -1; else if (value>100) value = 100;
              if (value>=0)
                print(images,0,"Set progress indice to %g%%.",
                      value);
              else
                print(images,0,"Disable progress indice.");
              *progress = (float)value;
            } else arg_error("progress");
            ++position; continue;
          }

          // Print.
          if (!is_double_hyphen && !std::strcmp("-print",command)) {
            print_images(images,images_names,selection);
            is_released = true; continue;
          }

          // Power.
          gmic_arithmetic_command("-pow",
                                  pow,
                                  "Compute image%s to the power of %g%s",
                                  gmic_selection.data(),value,ssep,Tfloat,
                                  pow,
                                  "Compute image%s to the power of image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute image%s to the power of expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute sequential power of image%s");

          // Draw point.
          if (!std::strcmp("-point",command)) {
            gmic_substitute_args();
            float x = 0, y = 0, z = 0;
            sepx = sepy = sepz = *argx = *argy = *argz = *color = 0;
            opacity = 1;
            if ((cimg_sscanf(argument,"%255[0-9.eE%+-]%c",
                             argx,&end)==1 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             argx,argy,&end)==2 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             argx,argy,argz,&end)==3 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],%f%c",
                             argx,argy,argz,&opacity,&end)==4 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],%f,"
                             "%4095[0-9.eEinfa,+-]%c",
                             argx,argy,argz,&opacity,color,&end)==5) &&
                (cimg_sscanf(argx,"%f%c",&x,&end)==1 ||
                 (cimg_sscanf(argx,"%f%c%c",&x,&sepx,&end)==2 && sepx=='%')) &&
                (!*argy ||
                 cimg_sscanf(argy,"%f%c",&y,&end)==1 ||
                 (cimg_sscanf(argy,"%f%c%c",&y,&sepy,&end)==2 && sepy=='%')) &&
                (!*argz ||
                 cimg_sscanf(argz,"%f%c",&z,&end)==1 ||
                 (cimg_sscanf(argz,"%f%c%c",&z,&sepz,&end)==2 && sepz=='%'))) {
              print(images,0,
                    "Draw point (%g%s,%g%s,%g%s) on image%s, with opacity %g and color (%s).",
                    x,sepx=='%'?"%":"",
                    y,sepy=='%'?"%":"",
                    z,sepz=='%'?"%":"",
                    gmic_selection.data(),
                    opacity,
                    *color?color:"default");
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                g_img.assign(img.spectrum(),1,1,1,0).fill(color,true,false);
                const int
                  nx = (int)cimg::round(sepx=='%'?x*(img.width() - 1)/100:x),
                  ny = (int)cimg::round(sepy=='%'?y*(img.height() - 1)/100:y),
                  nz = (int)cimg::round(sepz=='%'?z*(img.depth() - 1)/100:z);
                gmic_apply(draw_point(nx,ny,nz,g_img.data(),opacity));
              }
            } else arg_error("point");
            g_img.assign();
            is_released = false; ++position; continue;
          }

          // Draw polygon.
          if (!std::strcmp("-polygon",command)) {
            gmic_substitute_args();
            name.assign(256);
            float N = 0, x0 = 0, y0 = 0;
            sep1 = sepx = sepy = *name = *color = 0;
            pattern = ~0U; opacity = 1;
            if (cimg_sscanf(argument,"%f%c",
                            &N,&end)==2 && N>=1) {
              N = cimg::round(N);
              const char
                *nargument = argument + cimg_snprintf(name,name.width(),"%u",
                                                      (unsigned int)N) + 1,
                *const eargument = argument + std::strlen(argument);
              vertices.assign((unsigned int)N,2,1,1,0);
              CImg<bool> percents((unsigned int)N,2,1,1,0);
              for (unsigned int n = 0; n<(unsigned int)N; ++n) if (nargument<eargument) {
                  sepx = sepy = 0;
                  if (cimg_sscanf(nargument,"%255[0-9.eE%+-],%255[0-9.eE%+-]",
                                  argx,argy)==2 &&
                      (cimg_sscanf(argx,"%f%c",&x0,&end)==1 ||
                       (cimg_sscanf(argx,"%f%c%c",&x0,&sepx,&end)==2 && sepx=='%')) &&
                      (cimg_sscanf(argy,"%f%c",&y0,&end)==1 ||
                       (cimg_sscanf(argy,"%f%c%c",&y0,&sepy,&end)==2 && sepy=='%'))) {
                    vertices(n,0) = x0; percents(n,0) = (sepx=='%');
                    vertices(n,1) = y0; percents(n,1) = (sepy=='%');
                    nargument+=std::strlen(argx) + std::strlen(argy) + 2;
                  } else arg_error("polygon");
                } else arg_error("polygon");
              if (nargument<eargument &&
                  cimg_sscanf(nargument,"%4095[0-9.eEinfa+-]",color)==1 &&
                  cimg_sscanf(color,"%f",&opacity)==1) {
                nargument+=std::strlen(color) + 1;
                *color = 0;
              }
              if (nargument<eargument &&
                  cimg_sscanf(nargument,"0%c%4095[0-9a-fA-F]",&sep1,color)==2 && sep1=='x' &&
                  cimg_sscanf(color,"%x%c",&pattern,&end)==1) {
                nargument+=std::strlen(color) + 3;
                *color = 0;
              }
              const char *const _color = nargument<eargument?nargument:&(end=0);
              if (sep1=='x')
                print(images,0,"Draw %g-vertices outlined polygon on image%s, with opacity %g, "
                      "pattern 0x%x and color (%s).",
                      N,
                      gmic_selection.data(),
                      opacity,pattern,
                      *_color?_color:"default");
              else
                print(images,0,"Draw %g-vertices filled polygon on image%s, with opacity %g "
                      "and color (%s).",
                      N,
                      gmic_selection.data(),
                      opacity,
                      *_color?_color:"default");
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                CImg<int> coords(vertices.width(),2,1,1,0);
                cimg_forX(coords,p) {
                  if (percents(p,0))
                    coords(p,0) = (int)cimg::round(vertices(p,0)*(img.width() - 1)/100);
                  else coords(p,0) = (int)cimg::round(vertices(p,0));
                  if (percents(p,1))
                    coords(p,1) = (int)cimg::round(vertices(p,1)*(img.height() - 1)/100);
                  else coords(p,1) = (int)cimg::round(vertices(p,1));
                }
                g_img.assign(img.spectrum(),1,1,1,0).fill(_color,true,false);
                if (sep1=='x') { gmic_apply(draw_polygon(coords,g_img.data(),opacity,pattern)); }
                else gmic_apply(draw_polygon(coords,g_img.data(),opacity));
              }
            } else arg_error("polygon");
            vertices.assign(); g_img.assign();
            is_released = false; ++position; continue;
          }

          // Draw plasma fractal.
          if (!std::strcmp("-plasma",command)) {
            gmic_substitute_args();
            float alpha = 1, beta = 1, scale = 8;
            if ((cimg_sscanf(argument,"%f%c",
                             &alpha,&end)==1 ||
                 cimg_sscanf(argument,"%f,%f%c",
                             &alpha,&beta,&end)==2 ||
                 cimg_sscanf(argument,"%f,%f,%f%c",
                             &alpha,&beta,&scale,&end)==3) &&
                scale>=0) ++position;
            else { alpha = beta = 1; scale = 8; }
            const unsigned int _scale = (unsigned int)cimg::round(scale);
            print(images,0,"Draw plasma fractal on image%s, with alpha %g, beta %g and scale %u.",
                  gmic_selection.data(),
                  alpha,
                  beta,
                  _scale);
            cimg_forY(selection,l) gmic_apply(draw_plasma(alpha,beta,_scale));
            is_released = false; continue;
          }

          // Convert 3d object primitives.
          if (!std::strcmp("-primitives3d",command)) {
            gmic_substitute_args();
            unsigned int mode = 0;
            if (cimg_sscanf(argument,"%u%c",
                            &mode,&end)==1 &&
                mode<=2) {
              print(images,0,"Convert primitives of 3d object%s to %s.",
                    gmic_selection.data(),
                    mode==0?"points":mode==1?"segments":"no-textures");
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                CImg<T> &img = images[uind];
                try { gmic_apply(convert_primitives_CImg3d(mode)); }
                catch (CImgException&) {
                  if (!img.is_CImg3d(true,&(*message=0)))
                    error(images,0,0,
                          "Command '-primitives3d': Invalid 3d object [%d], "
                          "in selected image%s (%s).",
                          uind,gmic_selection_err.data(),message.data());
                  else throw;
                }
              }
            } else arg_error("primitives3d");
            is_released = false; ++position; continue;
          }

          // Get patch-match correspondence map.
          if (!std::strcmp("-patchmatch",command)) {
            gmic_substitute_args();
            float patch_width, patch_height, patch_depth = 1, nb_iterations = 5, nb_randoms = 5;
            const CImg<T> *initialization = 0;
            unsigned int is_score = 0;
            *argx = 0; ind0.assign();
            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%c",
                              indices,&patch_width,&end)==2 && (patch_height=patch_width)) ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%c",
                             indices,&patch_width,&patch_height,&end)==3 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f%c",
                             indices,&patch_width,&patch_height,&patch_depth,&end)==4 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f%c",
                             indices,&patch_width,&patch_height,&patch_depth,&nb_iterations,&end)==5 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f,%f%c",
                             indices,&patch_width,&patch_height,&patch_depth,&nb_iterations,&nb_randoms,&end)==6 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f,%f,%u%c",
                             indices,&patch_width,&patch_height,&patch_depth,&nb_iterations,&nb_randoms,
                             &is_score,&end)==7 ||
                 (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f,%f,%u,[%255[a-zA-Z0-9_.%+-]%c%c",
                              indices,&patch_width,&patch_height,&patch_depth,&nb_iterations,&nb_randoms,
                              &is_score,argx,&sep,&end)==9 && sep==']')) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-patchmatch",true,
                                    false,CImg<char>::empty())).height()==1 &&
                (!*argx ||
                 (ind0=selection2cimg(argx,images.size(),images_names,"-patchmatch",true,
                                      false,CImg<char>::empty())).height()==1) &&
                patch_width>=1 && patch_height>=1 && patch_depth>=1 &&
                nb_iterations>=0 && nb_randoms>=0 && is_score<=1) {
              patch_width = cimg::round(patch_width);
              patch_height = cimg::round(patch_height);
              patch_depth = cimg::round(patch_depth);
              nb_iterations = cimg::round(nb_iterations);
              nb_randoms = cimg::round(nb_randoms);
              if (ind0) initialization = &images[*ind0];
              print(images,0,"Estimate correspondence map between image%s and patch image [%u], "
                    "using %gx%gx%g patches, %g iteration%s, and %g randomization%s "
                    "(%sscore returned).",
                    gmic_selection.data(),
                    *ind,
                    patch_width,patch_height,patch_depth,
                    nb_iterations,nb_iterations!=1?"s":"",
                    nb_randoms,nb_randoms!=1?"s":"",
                    is_score?"":"no ");
              const CImg<T> patch_image = gmic_image_arg(*ind);
              cimg_forY(selection,l) gmic_apply(gmic_patchmatch(patch_image,
                                                                (unsigned int)patch_width,
                                                                (unsigned int)patch_height,
                                                                (unsigned int)patch_depth,
                                                                (unsigned int)nb_iterations,
                                                                (unsigned int)nb_randoms,
                                                                (bool)is_score,
                                                                initialization));
            } else arg_error("patchmatch");
            is_released = false; ++position; continue;
          }

          // Display as a graph plot.
          if (!is_double_hyphen && !std::strcmp("-plot",command)) {
            gmic_substitute_args();
            double ymin = 0, ymax = 0, xmin = 0, xmax = 0;
            unsigned int plot_type = 1, vertex_type = 1;
            float resolution = 65536;
            *formula = sep = 0;
            exit_on_anykey = 0;
            if (((cimg_sscanf(argument,"'%1023[^']%c%c",
                              formula,&sep,&end)==2 && sep=='\'') ||
                 cimg_sscanf(argument,"'%1023[^']',%f%c",
                             formula,&resolution,&end)==2 ||
                 cimg_sscanf(argument,"'%1023[^']',%f,%u%c",
                             formula,&resolution,&plot_type,&end)==3 ||
                 cimg_sscanf(argument,"'%1023[^']',%f,%u,%u%c",
                             formula,&resolution,&plot_type,&vertex_type,&end)==4 ||
                 cimg_sscanf(argument,"'%1023[^']',%f,%u,%u,%lf,%lf%c",
                             formula,&resolution,&plot_type,&vertex_type,&xmin,&xmax,&end)==6 ||
                 cimg_sscanf(argument,"'%1023[^']',%f,%u,%u,%lf,%lf,%lf,%lf%c",
                             formula,&resolution,&plot_type,&vertex_type,
                             &xmin,&xmax,&ymin,&ymax,&end)==8 ||
                 cimg_sscanf(argument,"'%1023[^']',%f,%u,%u,%lf,%lf,%lf,%lf,%u%c",
                             formula,&resolution,&plot_type,&vertex_type,
                             &xmin,&xmax,&ymin,&ymax,&exit_on_anykey,&end)==9) &&
                resolution>0 && plot_type<=3 && vertex_type<=7 && exit_on_anykey<=1) {
              resolution = cimg::round(resolution);
              strreplace_fw(formula);
              if (xmin==0 && xmax==0) { xmin = -4; xmax = 4; }
              if (!plot_type && !vertex_type) plot_type = 1;
              if (resolution<1) resolution = 65536;
              CImgList<double> tmp_img(1);
              CImg<double> &values = tmp_img[0];

              values.assign(4,(unsigned int)resolution--,1,1,0);
              const double dx = xmax - xmin;
              cimg_forY(values,X) values(0,X) = xmin + X*dx/resolution;
              cimg::eval(formula,values).move_to(values);

              cimg_snprintf(title,_title.width(),"[Plot of '%s']",formula);
              CImg<char>::string(title).move_to(g_list_c);
              display_plots(tmp_img,g_list_c,CImg<unsigned int>::vector(0),
                            plot_type,vertex_type,xmin,xmax,ymin,ymax,exit_on_anykey);
              g_list_c.assign();
              ++position;
            } else {
              plot_type = 1; vertex_type = 0; ymin = ymax = xmin = xmax = 0;
              if ((cimg_sscanf(argument,"%u%c",
                               &plot_type,&end)==1 ||
                   cimg_sscanf(argument,"%u,%u%c",
                               &plot_type,&vertex_type,&end)==2 ||
                   cimg_sscanf(argument,"%u,%u,%lf,%lf%c",
                               &plot_type,&vertex_type,&xmin,&xmax,&end)==4 ||
                   cimg_sscanf(argument,"%u,%u,%lf,%lf,%lf,%lf%c",
                               &plot_type,&vertex_type,&xmin,&xmax,&ymin,&ymax,&end)==6 ||
                   cimg_sscanf(argument,"%u,%u,%lf,%lf,%lf,%lf,%u%c",
                               &plot_type,&vertex_type,&xmin,&xmax,&ymin,&ymax,&exit_on_anykey,&end)==7) &&
                  plot_type<=3 && vertex_type<=7 && exit_on_anykey<=1) ++position;
              if (!plot_type && !vertex_type) plot_type = 1;
              display_plots(images,images_names,selection,plot_type,vertex_type,
                            xmin,xmax,ymin,ymax,exit_on_anykey);
            }
            is_released = true; continue;
          }

        } // command1=='p'.

        //----------------------------
        // Commands starting by '-q..'
        //----------------------------
        else if (command1=='q') {

          // Draw quiver.
          if (!std::strcmp("-quiver",command)) {
            gmic_substitute_args();
            float sampling = 25, factor = -20;
            unsigned int is_arrows = 1;
            pattern = ~0U; opacity = 1;
            sep = *color = 0;
            if ((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]]%c",
                             indices,&end)==1 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f%c",
                             indices,&sampling,&end)==2 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f%c",
                             indices,&sampling,&factor,&end)==3 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%u%c",
                             indices,&sampling,&factor,&is_arrows,&end)==4 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%u,%f%c",
                             indices,&sampling,&factor,&is_arrows,&opacity,&end)==5 ||
                 (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%u,%f,0%c%x%c",
                              indices,&sampling,&factor,&is_arrows,
                              &opacity,&sep,&pattern,&end)==7 && sep=='x') ||
                 (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%u,%f,%4095[0-9.eEinfa,+-]%c",
                              indices,&sampling,&factor,&is_arrows,&opacity,color,&end)==6 &&
                  (bool)(pattern=~0U)) ||
                 (*color=0,cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%u,"
                                       "%f,0%c%x,%4095[0-9.eEinfa,+-]%c",
                                       indices,&sampling,&factor,&is_arrows,
                                       &opacity,&sep,&pattern,color,&end)==8 &&
                  sep=='x')) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-quiver",true,
                                    false,CImg<char>::empty())).height()==1 &&
                sampling>0 && is_arrows<=1) {
              sampling = cimg::round(sampling);
              print(images,0,"Draw 2d vector field [%u] on image%s, with sampling %g, factor %g, "
                    "arrows %s, opacity %g, pattern 0x%x and color (%s).",
                    *ind,
                    gmic_selection.data(),
                    sampling,
                    factor,
                    is_arrows?"enabled":"disabled",
                    opacity,pattern,
                    *color?color:"default");
              const CImg<T> flow = gmic_image_arg(*ind);
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                g_img.assign(img.spectrum(),1,1,1,0).fill(color,true,false);
                gmic_apply(draw_quiver(flow,g_img.data(),opacity,(unsigned int)sampling,
                                       factor,(bool)is_arrows,pattern));
              }
            } else arg_error("quiver");
            g_img.assign();
            is_released = false; ++position; continue;
          }

        } // command1=='q'.

        //----------------------------
        // Commands starting by '-r..'
        //----------------------------
        else if (command1=='r') {

          // Remove images.
          if (!std::strcmp("-remove",command)) {
            print(images,0,"Remove image%s",
                  gmic_selection.data());
            if (is_double_hyphen) { g_list.assign(images); g_list_c.assign(images_names); }
            remove_images(images,images_names,selection,0,selection.height() - 1);
            if (is_double_hyphen) {
              g_list.move_to(images,0);
              g_list_c.move_to(images_names,0);
            }
            if (is_verbose) {
              cimg::mutex(29);
              std::fprintf(cimg::output()," (%u image%s left).",
                           images.size(),images.size()==1?"":"s");
              std::fflush(cimg::output());
              cimg::mutex(29,0);
            }
            g_list.assign(); g_list_c.assign();
            is_released = false; continue;
          }

          // Repeat.
          if (!std::strcmp("-repeat",item)) {
            gmic_substitute_args();
            float number = 0;
            *title  = 0;
            if (cimg_sscanf(argument,"%f%c",&number,&end)==1 ||
                (cimg_sscanf(argument,"%f,%255[a-zA-Z0-9_]%c",&number,title,&sep)==2 &&
                 (*title<'0' || *title>'9'))) {
              const unsigned int nb = number<=0?0U:
                cimg::type<float>::is_inf(number)?~0U:(unsigned int)cimg::round(number);
              if (nb) {
                if (is_debug_info && debug_line!=~0U) {
                  cimg_snprintf(argx,_argx.width(),"*repeat#%u",debug_line);
                  CImg<char>::string(argx).move_to(callstack);
                } else CImg<char>::string("*repeat").move_to(callstack);
                if (is_very_verbose) {
                  if (*title) print(images,0,"Start '-repeat..-done' block with variable '%s' (%u iteration%s).",
                                    title,nb,nb>1?"s":"");
                  else print(images,0,"Start '-repeat..-done' block (%u iteration%s).",
                             nb,nb>1?"s":"");
                }
                const unsigned int l = (unsigned int)std::strlen(title);
                CImg<unsigned int> rd(1,3 + (l?2:0));
                rd[0] = position + 1; rd[1] = nb; rd[2] = 0;
                if (l) {
                  const unsigned int hashcode = gmic::hashcode(title,true);
                  rd[3] = hashcode;
                  rd[4] = variables[hashcode]->_width;
                  CImg<char>::string(title).move_to(*variables_names[hashcode]);
                  CImg<char>::string("0").move_to(*variables[hashcode]);
                }
                rd.move_to(repeatdones);
              } else {
                if (is_very_verbose) {
                  if (*title) print(images,0,"Skip 'repeat..done' block with variable '%s' (0 iteration).",
                                    title);
                  else print(images,0,"Skip 'repeat..done' block (0 iteration).");
                }
                int nb_repeats = 0;
                for (nb_repeats = 1; nb_repeats && position<commands_line.size(); ++position) {
                  const char *it = commands_line[position].data();
                  if (!std::strcmp("-repeat",it)) ++nb_repeats;
                  else if (!std::strcmp("-done",it)) --nb_repeats;
                }
                if (nb_repeats && position>=commands_line.size())
                  error(images,0,0,
                        "Command '-repeat': Missing associated '-done' command.");
                continue;
              }
            } else arg_error("repeat");
            ++position; continue;
          }

          // Resize.
          if (!std::strcmp("-resize",command)) {
            gmic_substitute_args();
            float valx = 100, valy = 100, valz = 100, valc = 100, cx = 0, cy = 0, cz = 0, cc = 0;
            CImg<char> indicesy(256), indicesz(256), indicesc(256);
            CImg<unsigned int> ind, indx, indy, indz, indc;
            int interpolation = 1;
            *indices = *indicesy = *indicesz = *indicesc = *argx = *argy = *argz = *argc = sep = 0;
            sepx = sepy = sepz = sepc = '%';
            boundary = 0;

            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                              indices,&sep,&end)==2 && sep==']') ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%d%c",
                             indices,&interpolation,&end)==2 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%d,%u%c",
                             indices,&interpolation,&boundary,&end)==3 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%d,%u,%f%c",
                             indices,&interpolation,&boundary,&cx,&end)==4 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%d,%u,%f,%f%c",
                             indices,&interpolation,&boundary,&cx,&cy,&end)==5 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%d,%u,%f,%f,%f%c",
                             indices,&interpolation,&boundary,&cx,&cy,&cz,&end)==6 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%d,%u,%f,%f,%f,%f%c",
                             indices,&interpolation,&boundary,&cx,&cy,&cz,&cc,&end)==7) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-resize",true,
                                    false,CImg<char>::empty())).height()==1 &&
                interpolation>=-1 && interpolation<=6 && boundary<=2 &&
                cx>=0 && cx<=1 && cy>=0 && cy<=1 && cz>=0 && cz<=1 && cc>=0 && cc<=1) {
              const int
                nvalx = images[*ind].width(),
                nvaly = images[*ind].height(),
                nvalz = images[*ind].depth(),
                nvalc = images[*ind].spectrum();
              print(images,0,"Resize image%s to %dx%dx%dx%d, with %s interpolation, "
                    "%s boundary conditions and alignment (%g,%g,%g,%g).",
                    gmic_selection.data(),
                    nvalx,nvaly,nvalz,nvalc,
                    interpolation<=0?"no":interpolation==1?"nearest-neighbor":
                    interpolation==2?"moving average":interpolation==3?"linear":
                    interpolation==4?"grid":interpolation==5?"cubic":"lanczos",
                    boundary<=0?"dirichlet":boundary==1?"neumann":"periodic",
                    cx,cy,cz,cc);
              cimg_forY(selection,l) gmic_apply(resize(nvalx,nvaly,nvalz,nvalc,interpolation,boundary,cx,cy,cz,cc));
              ++position;
            } else if ((cx=cy=cz=cc=0, interpolation=1, boundary=0, true) &&
                       (cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-]%c",
                                    argx,&end)==1 ||
                        cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-]%c",
                                    argx,argy,&end)==2 ||
                        cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],"
                                    "%255[][a-zA-Z0-9_.eE%+-]%c",
                                    argx,argy,argz,&end)==3 ||
                        cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],"
                                    "%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-]%c",
                                    argx,argy,argz,argc,&end)==4 ||
                        cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],"
                                    "%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],%d%c",
                                    argx,argy,argz,argc,&interpolation,&end)==5 ||
                        cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],"
                                    "%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],%d,%u%c",
                                    argx,argy,argz,argc,&interpolation,&boundary,&end)==6 ||
                        cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],"
                                    "%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],%d,%u,%f%c",
                                    argx,argy,argz,argc,&interpolation,&boundary,&cx,&end)==7 ||
                        cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],"
                                    "%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],%d,%u,%f,"
                                    "%f%c",
                                    argx,argy,argz,argc,&interpolation,&boundary,&cx,&cy,&end)==8||
                        cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],"
                                    "%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],%d,%u,%f,"
                                    "%f,%f%c",
                                    argx,argy,argz,argc,&interpolation,&boundary,
                                    &cx,&cy,&cz,&end)==9 ||
                        cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],"
                                    "%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],%d,%u,%f,"
                                    "%f,%f,%f%c",
                                    argx,argy,argz,argc,&interpolation,&boundary,
                                    &cx,&cy,&cz,&cc,&end)==10) &&
                       ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sepx,&end)==2 &&
                         sepx==']' &&
                         (indx=selection2cimg(indices,images.size(),images_names,"-resize",true,
                                              false,CImg<char>::empty())).height()==1) ||
                        (sepx=0,cimg_sscanf(argx,"%f%c",&valx,&sepx)==1 && valx>=1) ||
                        (cimg_sscanf(argx,"%f%c%c",&valx,&sepx,&end)==2 && sepx=='%')) &&
                       (!*argy ||
                        (cimg_sscanf(argy,"[%255[a-zA-Z0-9_.%+-]%c%c",indicesy.data(),&sepy,
                                     &end)==2 &&
                         sepy==']' &&
                         (indy=selection2cimg(indicesy,images.size(),images_names,"-resize",true,
                                              false,CImg<char>::empty())).height()==1) ||
                        (sepy=0,cimg_sscanf(argy,"%f%c",&valy,&sepy)==1 && valy>=1) ||
                        (cimg_sscanf(argy,"%f%c%c",&valy,&sepy,&end)==2 && sepy=='%')) &&
                       (!*argz ||
                        (cimg_sscanf(argz,"[%255[a-zA-Z0-9_.%+-]%c%c",indicesz.data(),&sepz,
                                     &end)==2 &&
                         sepz==']' &&
                         (indz=selection2cimg(indicesz,images.size(),images_names,"-resize",true,
                                              false,CImg<char>::empty())).height()==1) ||
                        (sepz=0,cimg_sscanf(argz,"%f%c",&valz,&sepz)==1 && valz>=1) ||
                        (cimg_sscanf(argz,"%f%c%c",&valz,&sepz,&end)==2 && sepz=='%')) &&
                       (!*argc ||
                        (cimg_sscanf(argc,"[%255[a-zA-Z0-9_.%+-]%c%c",indicesc.data(),&sepc,
                                     &end)==2 &&
                         sepc==']' &&
                         (indc=selection2cimg(indicesc,images.size(),images_names,"-resize",true,
                                              false,CImg<char>::empty())).height()==1) ||
                        (sepc=0,cimg_sscanf(argc,"%f%c",&valc,&sepc)==1 && valc>=1) ||
                        (cimg_sscanf(argc,"%f%c%c",&valc,&sepc,&end)==2 && sepc=='%')) &&
                     valx>0 && valy>0 && valz>0 && valc>0 &&
                       interpolation>=-1 && interpolation<=6 && boundary<=2 &&
                       cx>=0 && cx<=1 && cy>=0 && cy<=1 && cz>=0 && cz<=1 && cc>=0 && cc<=1) {
              if (indx) { valx = (float)images[*indx].width(); sepx = 0; }
              if (indy) { valy = (float)images[*indy].height(); sepy = 0; }
              if (indz) { valz = (float)images[*indz].depth(); sepz = 0; }
              if (indc) { valc = (float)images[*indc].spectrum(); sepc = 0; }
              print(images,0,
                    "Resize image%s to %g%s%g%s%g%s%g%s, with %s interpolation, "
                    "%s boundary conditions and alignment (%g,%g,%g,%g).",
                    gmic_selection.data(),
                    valx,sepx=='%'?"%x":"x",
                    valy,sepy=='%'?"%x":"x",
                    valz,sepz=='%'?"%x":"x",
                    valc,sepc=='%'?"% ":"",
                    interpolation<=0?"no":interpolation==1?"nearest neighbor":
                    interpolation==2?"moving average":interpolation==3?"linear":
                    interpolation==4?"grid":interpolation==5?"cubic":"lanczos",
                    boundary<=0?"dirichlet":boundary==1?"neumann":"periodic",
                    cx,cy,cz,cc);
              cimg_forY(selection,l) {
                CImg<T>& img = images[selection[l]];
                const int
                  _nvalx = (int)cimg::round(sepx=='%'?valx*img.width()/100:valx),
                  _nvaly = (int)cimg::round(sepy=='%'?valy*img.height()/100:valy),
                  _nvalz = (int)cimg::round(sepz=='%'?valz*img.depth()/100:valz),
                  _nvalc = (int)cimg::round(sepc=='%'?valc*img.spectrum()/100:valc),
                  nvalx = _nvalx?_nvalx:1,
                  nvaly = _nvaly?_nvaly:1,
                  nvalz = _nvalz?_nvalz:1,
                  nvalc = _nvalc?_nvalc:1;
                gmic_apply(resize(nvalx,nvaly,nvalz,nvalc,interpolation,boundary,cx,cy,cz,cc));
              }
              ++position;
            } else {
#if cimg_display==0
              print(images,0,"Resize image%s in interactive mode (skipped, no display support).",
                    gmic_selection.data());
#else // #if cimg_display==0
              bool is_available_display = false;
              try {
                is_available_display = (bool)CImgDisplay::screen_width();
              } catch (CImgDisplayException&) {
                print(images,0,"Resize image%s in interactive mode (skipped, no display available).",
                      gmic_selection.data());
              }
              if (is_available_display) {
                print(images,0,"Resize image%s in interactive mode.",
                      gmic_selection.data());
                CImgDisplay _disp, &disp = _display_window[0]?_display_window[0]:_disp;
                cimg_forY(selection,l) {
                  CImg<T>& img = gmic_check(images[selection[l]]);
                  if (img) {
                    if (disp) disp.resize(cimg_fitscreen(img.width(),img.height(),1),false);
                    else disp.assign(cimg_fitscreen(img.width(),img.height(),1),0,1);
                    disp.set_title("%s: resize",basename(images_names[selection[l]].data()));
                    img.get_select(disp,0);
                    print(images,0,
                          "Resize image [%d] to %dx%d, with nearest-neighbor interpolation.",
                          selection[l],
                          disp.width(),
                          disp.height());
                    gmic_apply(resize(disp));
                  } else gmic_apply(replace(img));
                }
              }
#endif // #if cimg_display==0
            }
            is_released = false; continue;
          }

          // Reverse positions.
          if (!std::strcmp("-reverse",command)) {
            print(images,0,"Reverse positions of image%s.",
                  gmic_selection.data());
            if (is_double_hyphen) cimg_forY(selection,l) {
                const unsigned int i = selection[selection.height() - 1 - l];
                images.insert(images[i]);
                images_names.insert(images_names[i]);
              } else for (unsigned int l = 0; l<selection._height/2; ++l) {
                const unsigned int i0 = selection[l], i1 = selection[selection.height() - 1 - l];
                images[i0].swap(images[i1]);
                images_names[i0].swap(images_names[i1]);
              }
            is_released = false; continue;
          }

          // Return.
          if (!std::strcmp("-return",item)) {
            if (is_very_verbose) print(images,0,"Return.");
            position = commands_line.size();
            while (callstack && callstack.back()[0]=='*') {
              const char c = callstack.back()[1];
              if (c=='d') dowhiles.remove();
              else if (c=='r') repeatdones.remove();
              else if (c=='l' || c=='>' || c=='s') break;
              callstack.remove();
            }
            is_return = true;
            break;
          }

          // Keep rows.
          if (!std::strcmp("-rows",command)) {
            gmic_substitute_args();
            ind0.assign(); ind1.assign();
            sep0 = sep1 = *argx = *argy = 0;
            value0 = value1 = 0;
            if (cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-]%c",
                            argx,&end)==1 &&
                ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c]",indices,&sep0,&end)==2 &&
                  sep0==']' &&
                  (ind0=selection2cimg(indices,images.size(),images_names,"-rows",true,
                                       false,CImg<char>::empty())).height()==1) ||
                 cimg_sscanf(argx,"%lf%c",&value0,&end)==1 ||
                 (cimg_sscanf(argx,"%lf%c%c",&value0,&sep0,&end)==2 && sep0=='%'))) {
              if (ind0) { value0 = images[*ind0].height() - 1.; sep0 = 0; }
              print(images,0,"Keep rows %g%s of image%s.",
                    value0,sep0=='%'?"%":"",
                    gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                nvalue0 = cimg::round(sep0=='%'?value0*(img.height() - 1)/100:value0);
                gmic_apply(row((int)nvalue0));
              }
            } else if (cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-]%c",
                                   argx,argy,&end)==2 &&
                       ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep0,&end)==2 &&
                         sep0==']' &&
                         (ind0=selection2cimg(indices,images.size(),images_names,"-rows",true,
                                              false,CImg<char>::empty())).height()==1) ||
                        cimg_sscanf(argx,"%lf%c",&value0,&end)==1 ||
                        (cimg_sscanf(argx,"%lf%c%c",&value0,&sep0,&end)==2 && sep0=='%')) &&
                       ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c",formula,&sep0,&end)==2 &&
                         sep0==']' &&
                         (ind1=selection2cimg(formula,images.size(),images_names,"-rows",true,
                                              false,CImg<char>::empty())).height()==1) ||
                        cimg_sscanf(argy,"%lf%c",&value1,&end)==1 ||
                        (cimg_sscanf(argy,"%lf%c%c",&value1,&sep1,&end)==2 && sep1=='%'))) {
              if (ind0) { value0 = images[*ind0].height() - 1.; sep0 = 0; }
              if (ind1) { value1 = images[*ind1].height() - 1.; sep1 = 0; }
              print(images,0,"Keep rows %g%s...%g%s of image%s.",
                    value0,sep0=='%'?"%":"",
                    value1,sep1=='%'?"%":"",
                    gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                nvalue0 = cimg::round(sep0=='%'?value0*(img.height() - 1)/100:value0);
                nvalue1 = cimg::round(sep1=='%'?value1*(img.height() - 1)/100:value1);
                gmic_apply(rows((int)nvalue0,(int)nvalue1));
              }
            } else arg_error("rows");
            is_released = false; ++position; continue;
          }

          // Rotate.
          if (!std::strcmp("-rotate",command)) {
            gmic_substitute_args();
            float angle = 0, zoom = 1, cx = 0, cy = 0;
            unsigned int interpolation = 1;
            sep0 = sep1 = *argx = *argy = 0;
            boundary = 0;
            if ((cimg_sscanf(argument,"%f%c",
                             &angle,&end)==1 ||
                 cimg_sscanf(argument,"%f,%u%c",
                             &angle,&interpolation,&end)==2 ||
                 cimg_sscanf(argument,"%f,%u,%u%c",
                             &angle,&interpolation,&boundary,&end)==3 ||
                 cimg_sscanf(argument,"%f,%u,%u,%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             &angle,&interpolation,&boundary,argx,argy,&end)==5 ||
                 cimg_sscanf(argument,"%f,%u,%u,%255[0-9.eE%+-],%255[0-9.eE%+-],%f%c",
                             &angle,&interpolation,&boundary,argx,argy,&zoom,&end)==6) &&
                (!*argx ||
                 cimg_sscanf(argx,"%f%c",&cx,&end)==1 ||
                 (cimg_sscanf(argx,"%f%c%c",&cx,&sep0,&end)==2 && sep0=='%')) &&
                (!*argy ||
                 cimg_sscanf(argy,"%f%c",&cy,&end)==1 ||
                 (cimg_sscanf(argy,"%f%c%c",&cy,&sep1,&end)==2 && sep1=='%')) &&
                interpolation<=2 && boundary<=2) {
              if (*argx) {
                print(images,0,"Rotate image%s of %g°, %s interpolation, %s boundary conditions "
                      "with center at (%g%s,%g%s).",
                      gmic_selection.data(),angle,
                      interpolation==0?"nearest-neighbor":interpolation==1?"linear":"cubic",
                      boundary==0?"dirichlet":boundary==1?"neumann":"periodic",
                      cx,sep0=='%'?"%":"",cy,sep1=='%'?"%":"");
                cimg_forY(selection,l) {
                  CImg<T> &img = images[selection[l]];
                  const float
                    ncx = sep0=='%'?cx*(img.width() - 1)/100:cx,
                    ncy = sep1=='%'?cy*(img.height() - 1)/100:cy;
                  gmic_apply(rotate(angle,ncx,ncy,zoom,interpolation,boundary));
                }
              } else {
                print(images,0,"Rotate image%s of %g°, %s interpolation and %s boundary conditions.",
                      gmic_selection.data(),angle,
                      interpolation==0?"nearest-neighbor":interpolation==1?"linear":"cubic",
                      boundary==0?"dirichlet":boundary==1?"neumann":"periodic");
                cimg_forY(selection,l) gmic_apply(rotate(angle,interpolation,boundary));
              }
            } else arg_error("rotate");
            is_released = false; ++position; continue;
          }

          // Round.
          if (!std::strcmp("-round",command)) {
            gmic_substitute_args();
            int rounding_type = 0;
            value = 1;
            if ((cimg_sscanf(argument,"%lf%c",
                             &value,&end)==1 ||
                 cimg_sscanf(argument,"%lf,%d%c",
                             &value,&rounding_type,&end)==2) &&
                value>=0 && rounding_type>=-1 && rounding_type<=1) ++position;
            else { value = 1; rounding_type = 0; }
            print(images,0,"Round values of image%s by %g and %s rounding.",
                  gmic_selection.data(),
                  value,
                  rounding_type<0?"backward":rounding_type>0?"forward":"nearest");
            cimg_forY(selection,l) gmic_apply(round(value,rounding_type));
            is_released = false; continue;
          }

          // Fill with random values.
          if (!std::strcmp("-rand",command)) {
            gmic_substitute_args();
            ind0.assign(); ind1.assign();
            sep0 = sep1 = *argx = *argy = 0;
            value0 = value1 = 0;
            if (cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-]%c",
                            argx,argy,&end)==2 &&
                ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep0,&end)==2 &&
                  sep0==']' &&
                  (ind0=selection2cimg(indices,images.size(),images_names,"-rand",true,
                                       false,CImg<char>::empty())).height()==1) ||
                 (cimg_sscanf(argx,"%lf%c%c",&value0,&sep0,&end)==2 && sep0=='%') ||
                 cimg_sscanf(argx,"%lf%c",&value0,&end)==1) &&
                ((cimg_sscanf(argy,"[%255[a-zA-Z0-9_.%+-]%c%c",formula,&sep1,&end)==2 &&
                  sep1==']' &&
                  (ind1=selection2cimg(formula,images.size(),images_names,"-rand",true,
                                       false,CImg<char>::empty())).height()==1) ||
                 (cimg_sscanf(argy,"%lf%c%c",&value1,&sep1,&end)==2 && sep1=='%') ||
                 cimg_sscanf(argy,"%lf%c",&value1,&end)==1)) {
              if (ind0) { value0 = images[*ind0].min(); sep0 = 0; }
              if (ind1) { value1 = images[*ind1].max(); sep1 = 0; }
              print(images,0,"Fill image%s with random values, in range [%g%s,%g%s].",
                    gmic_selection.data(),
                    value0,sep0=='%'?"%":"",
                    value1,sep1=='%'?"%":"");
              cimg_forY(selection,l) {
                CImg<T>& img = gmic_check(images[selection[l]]);
                nvalue0 = value0; nvalue1 = value1;
                vmin = vmax = 0;
                if (sep0=='%' || sep1=='%') {
                  if (img) vmax = (double)img.max_min(vmin);
                  if (sep0=='%') nvalue0 = vmin + (vmax - vmin)*value0/100;
                  if (sep1=='%') nvalue1 = vmin + (vmax - vmin)*value1/100;
                }
                gmic_apply(rand((T)nvalue0,(T)nvalue1));
              }
            } else if (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep0,&end)==2 &&
                       sep0==']' &&
                       (ind0=selection2cimg(indices,images.size(),images_names,"-rand",true,
                                            false,CImg<char>::empty())).height()==1) {
              if (images[*ind0]) value1 = (double)images[*ind0].max_min(value0);
              print(images,0,"Fill image%s with random values, in range [%g,%g] from image [%d].",
                    gmic_selection.data(),
                    value0,
                    value1,
                    *ind0);
              cimg_forY(selection,l) gmic_apply(rand((T)value0,(T)value1));
            } else arg_error("rand");
            is_released = false; ++position; continue;
          }

          // Rotate 3d object.
          if (!std::strcmp("-rotate3d",command)) {
            gmic_substitute_args();
            float u = 0, v = 0, w = 1, angle = 0;
            if (cimg_sscanf(argument,"%f,%f,%f,%f%c",
                            &u,&v,&w,&angle,&end)==4) {
              print(images,0,"Rotate 3d object%s around axis (%g,%g,%g), with angle %g°.",
                    gmic_selection.data(),
                    u,v,w,
                    angle);
              CImg<float>::rotation_matrix(u,v,w,(float)(angle*cimg::PI/180)).move_to(vertices);
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                CImg<T>& img = images[uind];
                try { gmic_apply(rotate_CImg3d(vertices)); }
                catch (CImgException&) {
                  if (!img.is_CImg3d(true,&(*message=0)))
                    error(images,0,0,
                          "Command '-rotate3d': Invalid 3d object [%d], "
                          "in selected image%s (%s).",
                          uind,gmic_selection_err.data(),message.data());
                  else throw;
                }
              }
            } else arg_error("rotate3d");
            vertices.assign();
            is_released = false; ++position; continue;
          }

          // RGB to other color base.
          gmic_simple_command("-rgb2hsi",RGBtoHSI,"Convert image%s from RGB to HSI color bases.");
          gmic_simple_command("-rgb2hsl",RGBtoHSL,"Convert image%s from RGB to HSL color bases.");
          gmic_simple_command("-rgb2hsv",RGBtoHSV,"Convert image%s from RGB to HSV color bases.");
          gmic_simple_command("-rgb2lab",RGBtoLab,"Convert image%s from RGB to Lab color bases.");
          gmic_simple_command("-rgb2srgb",RGBtosRGB,"Convert image%s from RGB to sRGB color bases.");

          // Bitwise left rotation.
          gmic_arithmetic_command("-rol",
                                  rol,
                                  "Compute bitwise left rotation of image%s by %g%s",
                                  gmic_selection.data(),value,ssep,unsigned int,
                                  rol,
                                  "Compute bitwise left rotation of image%s by image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute bitwise left rotation of image%s by expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute sequential bitwise left rotation of image%s");

          // Bitwise right rotation.
          gmic_arithmetic_command("-ror",
                                  ror,
                                  "Compute bitwise right rotation of image%s by %g%s",
                                  gmic_selection.data(),value,ssep,unsigned int,
                                  ror,
                                  "Compute bitwise right rotation of image%s by image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute bitwise left rotation of image%s by expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute sequential bitwise left rotation of image%s");

          // Reverse 3d object orientation.
          if (!std::strcmp("-reverse3d",command)) {
            print(images,0,"Reverse orientation of 3d object%s.",
                  gmic_selection.data());
            cimg_forY(selection,l) {
              const unsigned int uind = selection[l];
              CImg<T> &img = images[uind];
              try { gmic_apply(reverse_CImg3d()); }
              catch (CImgException&) {
                if (!img.is_CImg3d(true,&(*message=0)))
                  error(images,0,0,
                        "Command '-reverse3d': Invalid 3d object [%d], "
                        "in selected image%s (%s).",
                        uind,gmic_selection_err.data(),message.data());
                else throw;
              }
            }
            is_released = false; continue;
          }

        } // command1=='r'.

        //----------------------------
        // Commands starting by '-s..'
        //----------------------------
        else if (command1=='s') {

          // Set status.
          if (!std::strcmp("-status",item)) {
            gmic_substitute_args();
            print(images,0,"Set status to '%s'.",gmic_argument_text_printed());
            CImg<char>::string(argument).move_to(status);
            ++position; continue;
          }

          // Skip argument.
          if (is_skip_command) {
            gmic_substitute_args();
            if (is_very_verbose)
              print(images,0,"Skip argument '%s'.",
                    gmic_argument_text_printed());
            ++position;
            continue;
          }

          // Set pixel value.
          if (!std::strcmp("-set",command)) {
            gmic_substitute_args();
            float x = 0, y = 0, z = 0, c = 0;
            value = 0;
            sepx = sepy = sepz = sepc = *argx = *argy = *argz = *argc = 0;
            if ((cimg_sscanf(argument,"%lf%c",
                             &value,&end)==1 ||
                 cimg_sscanf(argument,"%lf,%255[0-9.eE%+-]%c",
                             &value,argx,&end)==2 ||
                 cimg_sscanf(argument,"%lf,%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             &value,argx,argy,&end)==3 ||
                 cimg_sscanf(argument,"%lf,%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             &value,argx,argy,argz,&end)==4 ||
                 cimg_sscanf(argument,"%lf,%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-]%c",
                             &value,argx,argy,argz,argc,&end)==5) &&
                (!*argx ||
                 (cimg_sscanf(argx,"%f%c%c",&x,&sepx,&end)==2 && sepx=='%') ||
                 cimg_sscanf(argx,"%f%c",&x,&end)==1) &&
                (!*argy ||
                 (cimg_sscanf(argy,"%f%c%c",&y,&sepy,&end)==2 && sepy=='%') ||
                 cimg_sscanf(argy,"%f%c",&y,&end)==1) &&
                (!*argz ||
                 (cimg_sscanf(argz,"%f%c%c",&z,&sepz,&end)==2 && sepz=='%') ||
                 cimg_sscanf(argz,"%f%c",&z,&end)==1) &&
                (!*argc ||
                 (cimg_sscanf(argc,"%f%c%c",&c,&sepc,&end)==2 && sepc=='%') ||
                 cimg_sscanf(argc,"%f%c",&c,&end)==1)) {
              print(images,0,"Set value %g in image%s, at coordinates (%g%s,%g%s,%g%s,%g%s).",
                    value,
                    gmic_selection.data(),
                    x,sepx=='%'?"%":"",
                    y,sepy=='%'?"%":"",
                    z,sepz=='%'?"%":"",
                    c,sepc=='%'?"%":"");
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                const int
                  nx = (int)cimg::round(sepx=='%'?x*(img.width() - 1)/100:x),
                  ny = (int)cimg::round(sepy=='%'?y*(img.height() - 1)/100:y),
                  nz = (int)cimg::round(sepz=='%'?z*(img.depth() - 1)/100:z),
                  nc = (int)cimg::round(sepc=='%'?c*(img.spectrum() - 1)/100:c);
                gmic_apply(gmic_set(value,nx,ny,nz,nc));
              }
            } else arg_error("set");
            is_released = false; ++position; continue;
          }

          // Split.
          if (!std::strcmp("-split",command)) {
            bool is_valid_argument = false;
            gmic_substitute_args();
            float nb = -1;
            char pm = 0;
            *argx = 0;
            if (cimg_sscanf(argument,"%255[xyzc],%f%c",argx,&nb,&end)==2 ||
                (nb = -1,cimg_sscanf(argument,"%255[xyzc]%c",argx,&end))==1) {

              // Split along axes.
              nb = cimg::round(nb);
              if (nb>0)
                print(images,0,"Split image%s along the '%s'-ax%cs, into %g parts.",
                      gmic_selection.data(),
                      argx,
                      std::strlen(argx)>1?'e':'i',
                      nb);
              else if (nb<0) {
                if (nb==-1)
                  print(images,0,"Split image%s along the '%s'-ax%cs.",
                        gmic_selection.data(),
                        argx,
                        std::strlen(argx)>1?'e':'i');
                else
                  print(images,0,"Split image%s along the '%s'-ax%cs, into blocs of %g pixels.",
                        gmic_selection.data(),
                        argx,
                        std::strlen(argx)>1?'e':'i',
                        -nb);
              } else
                print(images,0,"Split image%s along the '%s'-ax%cs, according to constant values.",
                      gmic_selection.data(),
                      argx,
                      std::strlen(argx)>1?'e':'i');

              int off = 0;
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l] + off;
                const CImg<T>& img = gmic_check(images[uind]);
                if (!img) {
                  if (!is_double_hyphen) { images.remove(uind); images_names.remove(uind); off-=1; }
                } else {
                  g_list.assign(img,true);
                  name = images_names[uind];
                  for (const char *axis = argx; *axis; ++axis) {
                    const unsigned int N = g_list.size();
                    for (unsigned int l = 0; l<N; ++l) {
                      g_list[0].get_split(*axis,(int)nb).move_to(g_list,~0U);
                      g_list.remove(0);
                    }
                  }
                  if (is_double_hyphen) {
                    images_names.insert(g_list.size(),name.copymark());
                    g_list.move_to(images,~0U);
                  } else {
                    images.remove(uind); images_names.remove(uind);
                    off+=(int)g_list.size() - 1;
                    images_names.insert(g_list.size(),name.get_copymark(),uind);
                    name.move_to(images_names[uind]);
                    g_list.move_to(images,uind);
                  }
                }
              }
              g_list.assign();
              is_valid_argument = true;
            } else if (cimg_sscanf(argument,"%c%c",&pm,&end)==2 && (pm=='+' || pm=='-') && end==',') {

              // Split according to values sequence (opt. with axes too).
              const char *s_values = argument + 2;
              *argx = 0;
              if (cimg_sscanf(s_values,"%255[xyzc]%c",argx,&end)==2 && end==',') s_values+=std::strlen(argx) + 1;
              unsigned int nb_values = 1;
              for (const char *s = s_values; *s; ++s) if (*s==',') ++nb_values;
              CImg<T> values;
              try { values.assign(nb_values,1,1,1).fill(s_values,true,false); }
              catch (CImgException&) { values.assign(); }
              if (values) {
                if (*argx)
                  print(images,0,"Split image%s in '%s' mode along '%s'-ax%cs, according to value sequence '%s'.",
                        gmic_selection.data(),
                        pm=='-'?"discard":"keep",
                        argx,
                        std::strlen(argx)>1?'e':'i',
                        gmic_argument_text_printed() + (s_values - argument));
                else
                  print(images,0,"Split image%s in '%s' mode, according to value sequence '%s'.",
                        gmic_selection.data(),
                        pm=='-'?"discard":"keep",
                        gmic_argument_text_printed() + 2);
                int off = 0;
                cimg_forY(selection,l) {
                  const unsigned int uind = selection[l] + off;
                  const CImg<T>& img = gmic_check(images[uind]);
                  if (!img) {
                    if (!is_double_hyphen) { images.remove(uind); images_names.remove(uind); off-=1; }
                  } else {
                    name = images_names[uind];

                    if (*argx) { // Along axes.
                      g_list.assign(img,true);
                      for (const char *axis = argx; *axis; ++axis) {
                        const unsigned int N = g_list.size();
                        for (unsigned int l = 0; l<N; ++l) {
                          g_list[0].get_split(values,*axis,pm=='+').move_to(g_list,~0U);
                          g_list.remove(0);
                        }
                      }
                    } else // Without axes.
                      img.get_split(values,0,pm=='+').move_to(g_list);

                    if (is_double_hyphen) {
                      if (g_list) {
                        images_names.insert(g_list.size(),name.copymark());
                        g_list.move_to(images,~0U);
                      }
                    } else {
                      images.remove(uind);
                      images_names.remove(uind);
                      off+=(int)g_list.size() - 1;
                      if (g_list) {
                        images_names.insert(g_list.size(),name.get_copymark(),uind);
                        name.move_to(images_names[uind]);
                        g_list.move_to(images,uind);
                      }
                    }
                  }
                }
                g_list.assign();
                is_valid_argument = true;
              }
            }

            if (is_valid_argument) ++position;
            else {

              // Split as constant one-column vectors.
              print(images,0,"Split image%s as constant one-column vectors.",
                    gmic_selection.data());

              int off = 0;
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l] + off;
                CImg<T>& img = gmic_check(images[uind]);
                if (!img) {
                  if (!is_double_hyphen) { images.remove(uind); images_names.remove(uind); off-=1; }
                } else {
                  g_list = CImg<T>(img.data(),1,img.size(),1,1,true).get_split('y',0);
                  name = images_names[uind];
                  if (is_double_hyphen) {
                    images_names.insert(g_list.size(),name.copymark());
                    g_list.move_to(images,~0U);
                  } else {
                    images.remove(uind); images_names.remove(uind);
                    off+=(int)g_list.size() - 1;
                    images_names.insert(g_list.size(),name.get_copymark(),uind);
                    name.move_to(images_names[uind]);
                    g_list.move_to(images,uind);
                  }
                }
              }
            }
            g_list.assign();
            is_released = false; continue;
          }

          // Shared input.
          if (!std::strcmp("-shared",command)) {
            gmic_substitute_args();
            CImg<char> st0(256), st1(256), st2(256), st3(256), st4(256);
            char sep2 = 0, sep3 = 0, sep4 = 0;
            float a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0;
            sep0 = sep1 = *st0 = *st1 = *st2 = *st3 = *st4 = 0;
            if (cimg_sscanf(argument,
                            "%255[0-9.eE%+],%255[0-9.eE%+],%255[0-9.eE%+],%255[0-9.eE%+],"
                            "%255[0-9.eE%+]%c",
                            st0.data(),st1.data(),st2.data(),st3.data(),st4.data(),&end)==5 &&
                (cimg_sscanf(st0,"%f%c",&a0,&end)==1 ||
                 (cimg_sscanf(st0,"%f%c%c",&a0,&sep0,&end)==2 && sep0=='%')) &&
                (cimg_sscanf(st1,"%f%c",&a1,&end)==1 ||
                 (cimg_sscanf(st1,"%f%c%c",&a1,&sep1,&end)==2 && sep1=='%')) &&
                (cimg_sscanf(st2,"%f%c",&a2,&end)==1 ||
                 (cimg_sscanf(st2,"%f%c%c",&a2,&sep2,&end)==2 && sep2=='%')) &&
                (cimg_sscanf(st3,"%f%c",&a3,&end)==1 ||
                 (cimg_sscanf(st3,"%f%c%c",&a3,&sep3,&end)==2 && sep3=='%')) &&
                (cimg_sscanf(st4,"%f%c",&a4,&end)==1 ||
                 (cimg_sscanf(st4,"%f%c%c",&a4,&sep4,&end)==2 && sep4=='%'))) {
              print(images,0,
                    "Insert shared buffer%s from points (%g%s->%g%s,%g%s,%g%s,%g%s) of image%s.",
                    selection.height()>1?"s":"",
                    a0,sep0=='%'?"%":"",
                    a1,sep1=='%'?"%":"",
                    a2,sep2=='%'?"%":"",
                    a3,sep3=='%'?"%":"",
                    a4,sep4=='%'?"%":"",
                    gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T>& img = images[selection[l]];
                nvalue0 = cimg::round(sep0=='%'?a0*(img.width() - 1)/100:a0);
                nvalue1 = cimg::round(sep1=='%'?a1*(img.width() - 1)/100:a1);
                const unsigned int
                  y =  (unsigned int)cimg::round(sep2=='%'?a2*(img.height() - 1)/100:a2),
                  z =  (unsigned int)cimg::round(sep3=='%'?a3*(img.depth() - 1)/100:a3),
                  c =  (unsigned int)cimg::round(sep4=='%'?a4*(img.spectrum() - 1)/100:a4);
                images.insert(img.get_shared_points((unsigned int)nvalue0,(unsigned int)nvalue1,y,z,c),~0U,true);
                images_names.insert(images_names[selection[l]].get_copymark());
              }
              ++position;
            } else if (cimg_sscanf(argument,
                                   "%255[0-9.eE%+],%255[0-9.eE%+],%255[0-9.eE%+],"
                                   "%255[0-9.eE%+],%c",
                                   st0.data(),st1.data(),st2.data(),st3.data(),&end)==4 &&
                       (cimg_sscanf(st0,"%f%c",&a0,&end)==1 ||
                        (cimg_sscanf(st0,"%f%c%c",&a0,&sep0,&end)==2 && sep0=='%')) &&
                       (cimg_sscanf(st1,"%f%c",&a1,&end)==1 ||
                        (cimg_sscanf(st1,"%f%c%c",&a1,&sep1,&end)==2 && sep1=='%')) &&
                       (cimg_sscanf(st2,"%f%c",&a2,&end)==1 ||
                        (cimg_sscanf(st2,"%f%c%c",&a2,&sep2,&end)==2 && sep2=='%')) &&
                       (cimg_sscanf(st3,"%f%c",&a3,&end)==1 ||
                        (cimg_sscanf(st3,"%f%c%c",&a3,&sep3,&end)==2 && sep3=='%'))) {
              print(images,0,"Insert shared buffer%s from lines (%g%s->%g%s,%g%s,%g%s) of image%s.",
                    selection.height()>1?"s":"",
                    a0,sep0=='%'?"%":"",
                    a1,sep1=='%'?"%":"",
                    a2,sep2=='%'?"%":"",
                    a3,sep3=='%'?"%":"",
                    gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T>& img = images[selection[l]];
                nvalue0 = cimg::round(sep0=='%'?a0*(img.height() - 1)/100:a0);
                nvalue1 = cimg::round(sep1=='%'?a1*(img.height() - 1)/100:a1);
                const unsigned int
                  z =  (unsigned int)cimg::round(sep2=='%'?a2*(img.depth() - 1)/100:a2),
                  c =  (unsigned int)cimg::round(sep3=='%'?a3*(img.spectrum() - 1)/100:a3);
                images.insert(img.get_shared_rows((unsigned int)nvalue0,(unsigned int)nvalue1,z,c),~0U,true);
                images_names.insert(images_names[selection[l]].get_copymark());
              }
              ++position;
            } else if (cimg_sscanf(argument,"%255[0-9.eE%+],%255[0-9.eE%+],%255[0-9.eE%+]%c",
                                   st0.data(),st1.data(),st2.data(),&end)==3 &&
                       (cimg_sscanf(st0,"%f%c",&a0,&end)==1 ||
                        (cimg_sscanf(st0,"%f%c%c",&a0,&sep0,&end)==2 && sep0=='%')) &&
                       (cimg_sscanf(st1,"%f%c",&a1,&end)==1 ||
                        (cimg_sscanf(st1,"%f%c%c",&a1,&sep1,&end)==2 && sep1=='%')) &&
                       (cimg_sscanf(st2,"%f%c",&a2,&end)==1 ||
                        (cimg_sscanf(st2,"%f%c%c",&a2,&sep2,&end)==2 && sep2=='%'))) {
              print(images,0,"Insert shared buffer%s from planes (%g%s->%g%s,%g%s) of image%s.",
                    selection.height()>1?"s":"",
                    a0,sep0=='%'?"%":"",
                    a1,sep1=='%'?"%":"",
                    a2,sep2=='%'?"%":"",
                    gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T>& img = images[selection[l]];
                nvalue0 = cimg::round(sep0=='%'?a0*(img.depth() - 1)/100:a0);
                nvalue1 = cimg::round(sep1=='%'?a1*(img.depth() - 1)/100:a1);
                const unsigned int
                  c =  (unsigned int)cimg::round(sep2=='%'?a2*(img.spectrum() - 1)/100:a2);
                images.insert(img.get_shared_slices((unsigned int)nvalue0,(unsigned int)nvalue1,c),~0U,true);
                images_names.insert(images_names[selection[l]].get_copymark());
              }
              ++position;
            } else if (cimg_sscanf(argument,"%255[0-9.eE%+],%255[0-9.eE%+]%c",
                                   st0.data(),st1.data(),&end)==2 &&
                       (cimg_sscanf(st0,"%f%c",&a0,&end)==1 ||
                        (cimg_sscanf(st0,"%f%c%c",&a0,&sep0,&end)==2 && sep0=='%')) &&
                       (cimg_sscanf(st1,"%f%c",&a1,&end)==1 ||
                        (cimg_sscanf(st1,"%f%c%c",&a1,&sep1,&end)==2 && sep1=='%'))) {
              print(images,0,"Insert shared buffer%s from channels (%g%s->%g%s) of image%s.",
                    selection.height()>1?"s":"",
                    a0,sep0=='%'?"%":"",
                    a1,sep1=='%'?"%":"",
                    gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T>& img = images[selection[l]];
                nvalue0 = cimg::round(sep0=='%'?a0*(img.spectrum() - 1)/100:a0);
                nvalue1 = cimg::round(sep1=='%'?a1*(img.spectrum() - 1)/100:a1);
                images.insert(img.get_shared_channels((unsigned int)nvalue0,(unsigned int)nvalue1),~0U,true);
                images_names.insert(images_names[selection[l]].get_copymark());
              }
              ++position;
            } else if (cimg_sscanf(argument,"%255[0-9.eE%+]%c",
                                   st0.data(),&end)==1 &&
                       (cimg_sscanf(st0,"%f%c",&a0,&end)==1 ||
                        (cimg_sscanf(st0,"%f%c%c",&a0,&sep0,&end)==2 && sep0=='%'))) {
              print(images,0,"Insert shared buffer%s from channel %g%s of image%s.",
                    selection.height()>1?"s":"",
                    a0,sep0=='%'?"%":"",
                    gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T>& img = images[selection[l]];
                nvalue0 = cimg::round(sep0=='%'?a0*(img.spectrum() - 1)/100:a0);
                images.insert(img.get_shared_channel((unsigned int)nvalue0),~0U,true);
                images_names.insert(images_names[selection[l]].get_copymark());
              }
              ++position;
            } else {
              print(images,0,"Insert shared buffer%s from image%s.",
                    selection.height()>1?"s":"",
                    gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                images.insert(img,~0U,true);
                images_names.insert(images_names[selection[l]].get_copymark());
              }
            }
            is_released = false; continue;
          }

          // Shift.
          if (!std::strcmp("-shift",command)) {
            gmic_substitute_args();
            float dx = 0, dy = 0, dz = 0, dc = 0;
            sepx = sepy = sepz = sepc = *argx = *argy = *argz = *argc = 0;
            boundary = 0;
            if ((cimg_sscanf(argument,"%255[0-9.eE%+-]%c",
                             argx,&end)==1 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             argx,argy,&end)==2 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             argx,argy,argz,&end)==3 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-]%c",
                             argx,argy,argz,argc,&end)==4 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-],%u%c",
                             argx,argy,argz,argc,&boundary,&end)==5) &&
                (cimg_sscanf(argx,"%f%c",&dx,&end)==1 ||
                 (cimg_sscanf(argx,"%f%c%c",&dx,&sepx,&end)==2 && sepx=='%')) &&
                (!*argy ||
                 cimg_sscanf(argy,"%f%c",&dy,&end)==1 ||
                 (cimg_sscanf(argy,"%f%c%c",&dy,&sepy,&end)==2 && sepy=='%')) &&
                (!*argz ||
                 cimg_sscanf(argz,"%f%c",&dz,&end)==1 ||
                 (cimg_sscanf(argz,"%f%c%c",&dz,&sepz,&end)==2 && sepz=='%')) &&
                (!*argc ||
                 cimg_sscanf(argc,"%f%c",&dc,&end)==1 ||
                 (cimg_sscanf(argc,"%f%c%c",&dc,&sepc,&end)==2 && sepc=='%')) &&
                boundary<=2) {
              print(images,0,
                    "Shift image%s by displacement vector (%g%s,%g%s,%g%s,%g%s) and "
                    "%s boundary conditions.",
                    gmic_selection.data(),
                    dx,sepx=='%'?"%":"",
                    dy,sepy=='%'?"%":"",
                    dz,sepz=='%'?"%":"",
                    dc,sepc=='%'?"%":"",
                    boundary==0?"dirichlet":boundary==1?"neumann":"periodic");
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                const float
                  ndx = sepx=='%'?dx*img.width()/100:dx,
                  ndy = sepy=='%'?dy*img.height()/100:dy,
                  ndz = sepz=='%'?dz*img.depth()/100:dz,
                  ndc = sepc=='%'?dc*img.spectrum()/100:dc;
                /*                  ndx = cimg::round(sepx=='%'?dx*img.width()/100:dx),
                  ndy = cimg::round(sepy=='%'?dy*img.height()/100:dy),
                  ndz = cimg::round(sepz=='%'?dz*img.depth()/100:dz),
                  ndc = cimg::round(sepc=='%'?dc*img.spectrum()/100:dc);
                */
                gmic_apply(gmic_shift(ndx,ndy,ndz,ndc,boundary));
              }
            } else arg_error("shift");
            is_released = false; ++position; continue;
          }

          // Keep slices.
          if (!std::strcmp("-slices",command)) {
            gmic_substitute_args();
            ind0.assign(); ind1.assign();
            sep0 = sep1 = *argx = *argy = 0;
            value0 = value1 = 0;
            if (cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-]%c",
                            argx,&end)==1 &&
                ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c]",indices,&sep0,&end)==2 &&
                  sep0==']' &&
                  (ind0=selection2cimg(indices,images.size(),images_names,"-slices",true,
                                       false,CImg<char>::empty())).height()==1) ||
                 cimg_sscanf(argx,"%lf%c",&value0,&end)==1 ||
                 (cimg_sscanf(argx,"%lf%c%c",&value0,&sep0,&end)==2 && sep0=='%'))) {
              if (ind0) { value0 = images[*ind0].depth() - 1.; sep0 = 0; }
              print(images,0,"Keep slice %g%s of image%s.",
                    value0,sep0=='%'?"%":"",
                    gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                nvalue0 = cimg::round(sep0=='%'?value0*(img.depth() - 1)/100:value0);
                gmic_apply(slice((int)nvalue0));
              }
            } else if (cimg_sscanf(argument,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-]%c",
                                   argx,argy,&end)==2 &&
                       ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep0,&end)==2 &&
                         sep0==']' &&
                         (ind0=selection2cimg(indices,images.size(),images_names,"-slices",true,
                                              false,CImg<char>::empty())).height()==1) ||
                        cimg_sscanf(argx,"%lf%c",&value0,&end)==1 ||
                        (cimg_sscanf(argx,"%lf%c%c",&value0,&sep0,&end)==2 && sep0=='%')) &&
                       ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c",formula,&sep0,&end)==2 &&
                         sep0==']' &&
                         (ind1=selection2cimg(formula,images.size(),images_names,"-slices",true,
                                              false,CImg<char>::empty())).height()==1) ||
                        cimg_sscanf(argy,"%lf%c",&value1,&end)==1 ||
                        (cimg_sscanf(argy,"%lf%c%c",&value1,&sep1,&end)==2 && sep1=='%'))) {
              if (ind0) { value0 = images[*ind0].depth() - 1.; sep0 = 0; }
              if (ind1) { value1 = images[*ind1].depth() - 1.; sep1 = 0; }
              print(images,0,"Keep slices %g%s...%g%s of image%s.",
                    value0,sep0=='%'?"%":"",
                    value1,sep1=='%'?"%":"",
                    gmic_selection.data());
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                nvalue0 = cimg::round(sep0=='%'?value0*(img.depth() - 1)/100:value0);
                nvalue1 = cimg::round(sep1=='%'?value1*(img.depth() - 1)/100:value1);
                gmic_apply(slices((int)nvalue0,(int)nvalue1));
              }
            } else arg_error("slices");
            is_released = false; ++position; continue;
          }

          // Sub.
          gmic_arithmetic_command("-sub",
                                  operator-=,
                                  "Subtract %g%s to image%s",
                                  value,ssep,gmic_selection.data(),Tfloat,
                                  operator-=,
                                  "Subtract image [%d] to image%s",
                                  ind[0],gmic_selection.data(),
                                  "Subtract expression %s to image%s",
                                  gmic_argument_text_printed(),gmic_selection.data(),
                                  "Subtract image%s");
          // Square root.
          gmic_simple_command("-sqrt",sqrt,"Compute pointwise square root of image%s.");

          // Square.
          gmic_simple_command("-sqr",sqr,"Compute pointwise square function of image%s.");

          // Sign.
          gmic_simple_command("-sign",sign,"Compute pointwise sign of image%s.");

          // Sine.
          gmic_simple_command("-sin",sin,"Compute pointwise sine of image%s.");

          // Sort.
          if (!std::strcmp("-sort",command)) {
            gmic_substitute_args();
            char order = '+';
            axis = 0;
            if ((cimg_sscanf(argument,"%c%c",&order,&end)==1 ||
                 (cimg_sscanf(argument,"%c,%c%c",&order,&axis,&end)==2 &&
                  (axis=='x' || axis=='y' || axis=='z' || axis=='c'))) &&
                (order=='+' || order=='-')) ++position;
            else { order = '+'; axis = 0; }
            if (axis) print(images,0,"Sort values of image%s in %s order, according to axis '%c'.",
                            gmic_selection.data(),order=='+'?"ascending":"descending",axis);
            else print(images,0,"Sort values of image%s in %s order.",
                       gmic_selection.data(),order=='+'?"ascending":"descending");
            cimg_forY(selection,l) gmic_apply(sort(order=='+',axis));
            is_released = false; continue;
          }

          // Solve.
          if (!std::strcmp("-solve",command)) {
            gmic_substitute_args();
            sep = 0;
            if (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep,&end)==2 &&
                sep==']' &&
                (ind=selection2cimg(indices,images.size(),images_names,"-solve",true,
                                    false,CImg<char>::empty())).height()==1) {
              print(images,0,"Solve linear system AX = B, with B-vector%s and A-matrix [%d].",
                    gmic_selection.data(),*ind);
              const CImg<T> A = gmic_image_arg(*ind);
              cimg_forY(selection,l) gmic_apply(solve(A));
            } else arg_error("solve");
            is_released = false; ++position; continue;
          }

          // Shift 3d object, with opposite displacement.
          if (!std::strcmp("-sub3d",command)) {
            gmic_substitute_args();
            float tx = 0, ty = 0, tz = 0;
            if (cimg_sscanf(argument,"%f%c",
                            &tx,&end)==1 ||
                cimg_sscanf(argument,"%f,%f%c",
                            &tx,&ty,&end)==2 ||
                cimg_sscanf(argument,"%f,%f,%f%c",
                            &tx,&ty,&tz,&end)==3) {
              print(images,0,"Shift 3d object%s with displacement -(%g,%g,%g).",
                    gmic_selection.data(),
                    tx,ty,tz);
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                CImg<T>& img = images[uind];
                try { gmic_apply(shift_CImg3d(-tx,-ty,-tz)); }
                catch (CImgException&) {
                  if (!img.is_CImg3d(true,&(*message=0)))
                    error(images,0,0,
                          "Command '-sub3d': Invalid 3d object [%d], in selected image%s (%s).",
                          uind,gmic_selection_err.data(),message.data());
                  else throw;
                }
              }
            } else arg_error("sub3d");
            is_released = false; ++position; continue;
          }

          // Sharpen.
          if (!std::strcmp("-sharpen",command)) {
            gmic_substitute_args();
            float amplitude = 0, edge = -1, alpha = 0, sigma = 0;
            if ((cimg_sscanf(argument,"%f%c",
                             &amplitude,&end)==1 ||
                 cimg_sscanf(argument,"%f,%f%c",
                             &amplitude,&edge,&end)==2 ||
                 cimg_sscanf(argument,"%f,%f,%f%c",
                             &amplitude,&edge,&alpha,&end)==3 ||
                 cimg_sscanf(argument,"%f,%f,%f,%f%c",
                             &amplitude,&edge,&alpha,&sigma,&end)==4) &&
                amplitude>=0 && (edge==-1 || edge>=0)) {
              if (edge>=0)
                print(images,0,"Sharpen image%s with shock filters, amplitude %g, edge %g, "
                      "alpha %g and sigma %g.",
                      gmic_selection.data(),
                      amplitude,
                      edge,
                      alpha,
                      sigma);
              else
                print(images,0,"Sharpen image%s with inverse diffusion and amplitude %g.",
                      gmic_selection.data(),
                      amplitude);
              cimg_forY(selection,l) gmic_apply(sharpen(amplitude,(bool)(edge>=0),edge,alpha,sigma));
            } else arg_error("sharpen");
            is_released = false; ++position; continue;
          }

          // Set random generator seed.
          if (!std::strcmp("-srand",item)) {
            gmic_substitute_args();
            value = 0;
            if (cimg_sscanf(argument,"%lf%c",
                            &value,&end)==1) {
              value = cimg::round(value);
              print(images,0,"Set random generator seed to %u.",
                    (unsigned int)value);
              cimg::srand((unsigned int)value);
              ++position;
            } else {
              print(images,0,"Set random generator seed to random.");
              cimg::srand();
            }
            continue;
          }

          // Anisotropic PDE-based smoothing.
          if (!std::strcmp("-smooth",command)) {
            gmic_substitute_args();
            float amplitude = 0, sharpness = 0.7f, anisotropy = 0.3f, alpha = 0.6f,
              sigma = 1.1f, dl =0.8f, da = 30.0f, gauss_prec = 2.0f;
            unsigned int interpolation = 0, is_fast_approximation = 1;
            sep = 0;
            if ((cimg_sscanf(argument,"%f%c",
                             &amplitude,&end)==1 ||
                 cimg_sscanf(argument,"%f,%f%c",
                             &amplitude,&sharpness,&end)==2 ||
                 cimg_sscanf(argument,"%f,%f,%f%c",
                             &amplitude,&sharpness,&anisotropy,&end)==3 ||
                 cimg_sscanf(argument,"%f,%f,%f,%f%c",
                             &amplitude,&sharpness,&anisotropy,&alpha,&end)==4 ||
                 cimg_sscanf(argument,"%f,%f,%f,%f,%f%c",
                             &amplitude,&sharpness,&anisotropy,&alpha,&sigma,&end)==5 ||
                 cimg_sscanf(argument,"%f,%f,%f,%f,%f,%f%c",
                             &amplitude,&sharpness,&anisotropy,&alpha,&sigma,&dl,&end)==6 ||
                 cimg_sscanf(argument,"%f,%f,%f,%f,%f,%f,%f%c",
                             &amplitude,&sharpness,&anisotropy,&alpha,&sigma,&dl,&da,&end)==7 ||
                 cimg_sscanf(argument,"%f,%f,%f,%f,%f,%f,%f,%f%c",
                             &amplitude,&sharpness,&anisotropy,&alpha,&sigma,&dl,&da,&gauss_prec,
                             &end)==8 ||
                 cimg_sscanf(argument,"%f,%f,%f,%f,%f,%f,%f,%f,%u%c",
                             &amplitude,&sharpness,&anisotropy,&alpha,&sigma,&dl,&da,&gauss_prec,
                             &interpolation,&end)==9 ||
                 cimg_sscanf(argument,"%f,%f,%f,%f,%f,%f,%f,%f,%u,%u%c",
                             &amplitude,&sharpness,&anisotropy,&alpha,&sigma,&dl,&da,&gauss_prec,
                             &interpolation,&is_fast_approximation,&end)==10) &&
                amplitude>=0 && sharpness>=0 && anisotropy>=0 && anisotropy<=1 && dl>0 &&
                da>=0 && gauss_prec>0 && interpolation<=2 && is_fast_approximation<=1) {
              if (da>0)
                print(images,0,"Smooth image%s anisotropically, with amplitude %g, sharpness %g, "
                      "anisotropy %g, alpha %g, sigma %g, dl %g, da %g, precision %g, "
                      "%s interpolation and fast approximation %s.",
                      gmic_selection.data(),
                      amplitude,
                      sharpness,
                      anisotropy,
                      alpha,
                      sigma,
                      dl,
                      da,
                      gauss_prec,
                      interpolation==0?"nearest-neighbor":interpolation==1?"linear":"runge-kutta",
                      is_fast_approximation?"enabled":"disabled");
              else
                print(images,0,"Smooth image%s anisotropically, with %d iterations, sharpness %g, "
                      "anisotropy %g, alpha %g, sigma %g and dt %g.",
                      gmic_selection.data(),
                      (int)amplitude,
                      sharpness,
                      anisotropy,
                      alpha,
                      sigma,
                      dl);
              cimg_forY(selection,l) gmic_apply(blur_anisotropic(amplitude,sharpness,anisotropy,alpha,sigma,dl,da,
                                                                 gauss_prec,interpolation,(bool)is_fast_approximation));
            } else if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                                     indices,&sep,&end)==2 && sep==']') ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f%c",
                                    indices,&amplitude,&end)==2 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f%c",
                                    indices,&amplitude,&dl,&end)==3 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f%c",
                                    indices,&amplitude,&dl,&da,&end)==4 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f%c",
                                    indices,&amplitude,&dl,&da,&gauss_prec,&end)==5 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f,%u%c",
                                    indices,&amplitude,&dl,&da,&gauss_prec,
                                    &interpolation,&end)==6 ||
                        cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%f,%f,%f,%f,%u,%u%c",
                                    indices,&amplitude,&dl,&da,&gauss_prec,&interpolation,
                                    &is_fast_approximation,&end)==7) &&
                       (ind=selection2cimg(indices,images.size(),images_names,"-smooth",true,
                                           false,CImg<char>::empty())).height()==1 &&
                       amplitude>=0 && dl>0 && da>=0 && gauss_prec>0 && interpolation<=2 &&
                       is_fast_approximation<=1) {
              const CImg<T> tensors = gmic_image_arg(*ind);
              if (da>0)
                print(images,0,
                      "Smooth image%s anisotropically, with tensor field [%u], amplitude %g, "
                      "dl %g, da %g, precision %g, %s interpolation and fast approximation %s.",
                      gmic_selection.data(),
                      *ind,
                      amplitude,
                      dl,
                      da,
                      gauss_prec,
                      interpolation==0?"nearest-neighbor":interpolation==1?"linear":"runge-kutta",
                      is_fast_approximation?"enabled":"disabled");
              else
                print(images,0,
                      "Smooth image%s anisotropically, with tensor field [%u], %d iterations "
                      "and dt %g.",
                      gmic_selection.data(),
                      *ind,
                      (int)amplitude,
                      dl);
              cimg_forY(selection,l) gmic_apply(blur_anisotropic(tensors,amplitude,dl,da,gauss_prec,interpolation,
                                                                 is_fast_approximation));
            } else arg_error("smooth");
            is_released = false; ++position; continue;
          }

          // Split 3d objects, into 6 vector images
          // { header,N,vertices,primitives,colors,opacities }
          if (!std::strcmp("-split3d",command)) {
            bool keep_shared = true;
            gmic_substitute_args();
            if ((*argument=='0' || *argument=='1') && !argument[1]) {
              keep_shared = *argument=='1';
              ++position;
            }
            print(images,0,"Split 3d object%s into 6 property vectors%s.",
                  gmic_selection.data(),
                  keep_shared?"":" and clone shared data");
            unsigned int off = 0;
            cimg_forY(selection,l) {
              const unsigned int uind = selection[l] + off;
              const CImg<T> &img = gmic_check(images[uind]);
              name = images_names[uind];
              try {
                if (!keep_shared) {
                  CImg<T> _vertices;
                  CImgList<T> Tcolors, opacities;
                  img.get_CImg3dtoobject3d(primitives,Tcolors,opacities,false).move_to(_vertices);
                  CImgList<T> _colors(Tcolors,false), _opacities(opacities,false);
                  _colors.move_to(Tcolors.assign());
                  _opacities.move_to(opacities.assign());
                  _vertices.object3dtoCImg3d(primitives,Tcolors,opacities,false).get_split_CImg3d().
                    move_to(g_list);
                  primitives.assign();
                } else img.get_split_CImg3d().move_to(g_list);
              } catch (CImgException&) {
                if (!img.is_CImg3d(true,&(*message=0)))
                  error(images,0,0,
                        "Command '-split3d': Invalid 3d object [%d], in selected image%s (%s).",
                        uind - off,gmic_selection_err.data(),message.data());
                else throw;
              }
              if (is_double_hyphen) {
                images_names.insert(g_list.size(),name.copymark());
                g_list.move_to(images,~0U);
              } else {
                images.remove(uind);
                images_names.remove(uind);
                off+=g_list.size() - 1;
                images_names.insert(g_list.size(),name.get_copymark(),uind);
                name.move_to(images_names[uind]);
                g_list.move_to(images,uind);
              }
            }
            g_list.assign();
            is_released = false; continue;
          }

          // SVD.
          if (!std::strcmp("-svd",command)) {
            print(images,0,"Compute SVD decomposition%s of matri%s%s.",
                  selection.height()>1?"s":"",selection.height()>1?"ce":"x",gmic_selection.data());
            CImg<float> U, S, V;
            unsigned int off = 0;
            cimg_forY(selection,l) {
              const unsigned int uind = selection[l] + off;
              const CImg<T>& img = gmic_check(images[uind]);
              name = images_names[uind];
              img.SVD(U,S,V,true,100);
              if (is_double_hyphen) {
                images_names.insert(2,name.copymark());
                name.move_to(images_names);
                U.move_to(images);
                S.move_to(images);
                V.move_to(images);
              } else {
                images_names.insert(2,name.get_copymark(),uind + 1);
                name.move_to(images_names[uind]);
                U.move_to(images[uind].assign());
                images.insert(S,uind + 1);
                images.insert(V,uind + 2);
                off+=2;
              }
            }
            is_released = false; continue;
          }

          // Input 3d sphere.
          if (!std::strcmp("-sphere3d",item)) {
            gmic_substitute_args();
            float radius = 100, recursions = 3;
            if ((cimg_sscanf(argument,"%f%c",
                             &radius,&end)==1 ||
                 cimg_sscanf(argument,"%f,%f%c",
                             &radius,&recursions,&end)==2) &&
                recursions>=0) {
              recursions = cimg::round(recursions);
              print(images,0,"Input 3d sphere, with radius %g and %g recursions.",
                    radius,
                    recursions);
              CImg<T>::sphere3d(primitives,radius,(unsigned int)recursions).move_to(vertices);
              vertices.object3dtoCImg3d(primitives,false).move_to(images);
              primitives.assign();
              CImg<char>::string("[3d sphere]").move_to(images_names);
            } else arg_error("sphere3d");
            is_released = false; ++position; continue;
          }

          // Set 3d specular light parameters.
          if (!std::strcmp("-specl3d",item)) {
            gmic_substitute_args();
            value = 0.15;
            if (cimg_sscanf(argument,"%lf%c",
                            &value,&end)==1 && value>=0) ++position;
            else value = 0.15;
            specular_lightness3d = (float)value;
            print(images,0,"Set lightness of 3d specular light to %g.",
                  specular_lightness3d);
            continue;
          }

          if (!std::strcmp("-specs3d",item)) {
            gmic_substitute_args();
            value = 0.8;
            if (cimg_sscanf(argument,"%lf%c",
                            &value,&end)==1 && value>=0) ++position;
            else value = 0.8;
            specular_shininess3d = (float)value;
            print(images,0,"Set shininess of 3d specular light to %g.",
                  specular_shininess3d);
            continue;
          }

          // Sine-cardinal.
          gmic_simple_command("-sinc",sinc,"Compute pointwise sinc function of image%s.");

          // Hyperbolic sine.
          gmic_simple_command("-sinh",sinh,"Compute pointwise hyperpolic sine of image%s.");

          // sRGB to RGB.
          gmic_simple_command("-srgb2rgb",sRGBtoRGB,"Convert image%s from sRGB to RGB color bases.");

          // Extract 3d streamline.
          if (!std::strcmp("-streamline3d",command)) {
            gmic_substitute_args();
            unsigned int interp = 2, is_backward = 0, is_oriented_only = 0;
            float x = 0, y = 0, z = 0, L = 100, dl = 0.1f;
            sepx = sepy = sepz = *formula = 0;
            if ((cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             argx,argy,argz,&end)==3 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],%f%c",
                             argx,argy,argz,&L,&end)==4 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],%f,%f%c",
                             argx,argy,argz,&L,&dl,&end)==5 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],%f,%f,%u%c",
                             argx,argy,argz,&L,&dl,&interp,&end)==6 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],%f,%f,%u,"
                             "%u%c",
                             argx,argy,argz,&L,&dl,&interp,&is_backward,&end)==7 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],%f,%f,%u,"
                             "%u,%u%c",
                             argx,argy,argz,&L,&dl,&interp,&is_backward,
                             &is_oriented_only,&end)==8) &&
                (cimg_sscanf(argx,"%f%c",&x,&end)==1 ||
                 (cimg_sscanf(argx,"%f%c%c",&x,&sepx,&end)==2 && sepx=='%')) &&
                (!*argy ||
                 cimg_sscanf(argy,"%f%c",&y,&end)==1 ||
                 (cimg_sscanf(argy,"%f%c%c",&y,&sepy,&end)==2 && sepy=='%')) &&
                (!*argz ||
                 cimg_sscanf(argz,"%f%c",&z,&end)==1 ||
                 (cimg_sscanf(argz,"%f%c%c",&z,&sepz,&end)==2 && sepz=='%')) &&
                L>=0 && dl>0 && interp<4 && is_backward<=1 && is_oriented_only<=1) {
              print(images,0,"Extract 3d streamline from image%s, starting from (%g%s,%g%s,%g%s).",
                    gmic_selection.data(),
                    x,sepx=='%'?"%":"",
                    y,sepy=='%'?"%":"",
                    z,sepz=='%'?"%":"");
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                CImg<T>& img = gmic_check(images[uind]);
                const float
                  nx = cimg::round(sepx=='%'?x*(img.width() - 1)/100:x),
                  ny = cimg::round(sepy=='%'?y*(img.height() - 1)/100:y),
                  nz = cimg::round(sepz=='%'?z*(img.depth() - 1)/100:z);
                img.get_streamline(nx,ny,nz,L,dl,interp,
                                   (bool)is_backward,(bool)is_oriented_only).move_to(vertices);
                if (vertices.width()>1) {
                  primitives.assign(vertices.width() - 1,1,2);
                  cimglist_for(primitives,l) { primitives(l,0) = (unsigned int)l; primitives(l,1) = l + 1U; }
                  g_list_uc.assign(primitives.size(),1,3,1,1,200);
                } else {
                  vertices.assign();
                  warn(images,0,false,
                       "Command '-streamline3d': Empty streamline starting from "
                       "(%g%s,%g%s,%g%s) in image [%u].",
                       x,sepx=='%'?"%":"",
                       y,sepy=='%'?"%":"",
                       z,sepz=='%'?"%":"",
                       uind);
                }
                vertices.object3dtoCImg3d(primitives,g_list_uc,false);
                gmic_apply(replace(vertices));
                primitives.assign();
                g_list_uc.assign();
              }
            } else if ((cimg_sscanf(argument,"'%4095[^']',%f,%f,%f%c",
                                    formula,&x,&y,&z,&end)==4 ||
                        cimg_sscanf(argument,"'%4095[^']',%f,%f,%f,%f%c",
                                    formula,&x,&y,&z,&L,&end)==5 ||
                        cimg_sscanf(argument,"'%4095[^']',%f,%f,%f,%f,%f%c",
                                    formula,&x,&y,&z,&L,&dl,&end)==6 ||
                        cimg_sscanf(argument,"'%4095[^']',%f,%f,%f,%f,%f,%u%c",
                                    formula,&x,&y,&z,&L,&dl,&interp,&end)==7 ||
                        cimg_sscanf(argument,"'%4095[^']',%f,%f,%f,%f,%f,%u,%u%c",
                                    formula,&x,&y,&z,&L,&dl,&interp,&is_backward,&end)==8 ||
                        cimg_sscanf(argument,"'%4095[^']',%f,%f,%f,%f,%f,%u,%u,%u%c",
                                    formula,&x,&y,&z,&L,&dl,&interp,&is_backward,
                                    &is_oriented_only,&end)==9) &&
                       dl>0 && interp<4) {
              strreplace_fw(formula);
              print(images,0,"Extract 3d streamline from formula '%s', starting from (%g,%g,%g).",
                    formula,
                    x,y,z);
              CImg<T>::streamline((const char *)formula,x,y,z,L,dl,interp,
                                  (bool)is_backward,(bool)is_oriented_only).move_to(vertices);
              if (vertices.width()>1) {
                primitives.assign(vertices.width() - 1,1,2);
                cimglist_for(primitives,l) { primitives(l,0) = (unsigned int)l; primitives(l,1) = l + 1U; }
                g_list_uc.assign(primitives.size(),1,3,1,1,200);
              } else {
                vertices.assign();
                warn(images,0,false,
                     "Command '-streamline3d': Empty streamline starting from (%g,%g,%g) "
                     "in expression '%s'.",
                     x,y,z,formula);
              }
              vertices.object3dtoCImg3d(primitives,g_list_uc,false).move_to(images);
              primitives.assign();
              g_list_uc.assign();
              cimg_snprintf(title,_title.width(),"[3d streamline of '%s' at (%g,%g,%g)]",
                            formula,x,y,z);
              CImg<char>::string(title).move_to(images_names);
            } else arg_error("streamline3d");
            is_released = false; ++position; continue;
          }

          // Compute structure tensor field.
          if (!std::strcmp("-structuretensors",command)) {
            gmic_substitute_args();
            unsigned int is_fwbw_scheme = 0;
            if (cimg_sscanf(argument,"%u%c",&is_fwbw_scheme,&end)==1 &&
                is_fwbw_scheme<=1) ++position;
            else is_fwbw_scheme = 1;
            print(images,0,"Compute structure tensor field of image%s, with %s scheme.",
                  gmic_selection.data(),
                  is_fwbw_scheme==1?"forward-backward":"centered");
            cimg_forY(selection,l) gmic_apply(structure_tensors((bool)is_fwbw_scheme));
            is_released = false; continue;
          }

          // Select image feature.
          if (!std::strcmp("-select",command)) {
            gmic_substitute_args();
            unsigned int feature_type = 0, X = ~0U, Y = ~0U, Z = ~0U;
            bool is_xyz = false;
            exit_on_anykey = 0;
            if ((cimg_sscanf(argument,"%u%c",&feature_type,&end)==1 ||
                 (is_xyz=(cimg_sscanf(argument,"%u,%u,%u,%u%c",&feature_type,&X,&Y,&Z,&end)==4)) ||
                 (is_xyz=(cimg_sscanf(argument,"%u,%u,%u,%u,%u%c",&feature_type,&X,&Y,&Z,&exit_on_anykey,&end)==5))) &&
                feature_type<=3 && exit_on_anykey<=1) {
#if cimg_display==0
              print(images,0,"Select %s in image%s in interactive mode",
                    feature_type==0?"point":feature_type==1?"segment":feature_type==2?"rectangle":
                    "ellipse",gmic_selection.data());
              if (is_verbose) {
                cimg::mutex(29);
                if (is_xyz) std::fprintf(cimg::output(),", from point (%u,%u,%u)",X,Y,Z);
                std::fprintf(cimg::output()," (skipped, no display support).");
                std::fflush(cimg::output());
                cimg::mutex(29,0);
              }
              CImg<char>::string("-1").move_to(status);
#else // #if cimg_display==0
              bool is_available_display = false;
              try {
                is_available_display = (bool)CImgDisplay::screen_width();
              } catch (CImgDisplayException&) {
                print(images,0,
                      "Select %s in image%s in interactive mode",
                      feature_type==0?"point":feature_type==1?"segment":
                      feature_type==2?"rectangle":"ellipse",gmic_selection.data());
                if (is_verbose) {
                  cimg::mutex(29);
                  if (is_xyz) std::fprintf(cimg::output(),", from point (%u,%u,%u)",X,Y,Z);
                  std::fprintf(cimg::output()," (skipped, no display available).");
                  std::fflush(cimg::output());
                  cimg::mutex(29,0);
                }
              }
              if (is_available_display) {
                print(images,0,"Select %s in image%s in interactive mode",
                      feature_type==0?"point":feature_type==1?"segment":
                      feature_type==2?"rectangle":"ellipse",gmic_selection.data());
                if (is_verbose) {
                  cimg::mutex(29);
                  if (is_xyz) std::fprintf(cimg::output(),", from point (%u,%u,%u).",X,Y,Z);
                  else std::fprintf(cimg::output(),".");
                  std::fflush(cimg::output());
                  cimg::mutex(29,0);
                }
                unsigned int XYZ[3];
                XYZ[0] = X; XYZ[1] = Y; XYZ[2] = Z;
                if (_display_window[0]) {
                  cimg_forY(selection,l)
                    gmic_apply(select(_display_window[0],feature_type,is_xyz?XYZ:0,
                                      (bool)exit_on_anykey));
                }
                else {
                  cimg_forY(selection,l)
                    gmic_apply(select(images_names[selection[l]].data(),feature_type,is_xyz?XYZ:0,
                                      (bool)exit_on_anykey));
                }
              }
              if (is_double_hyphen) images.back().value_string().move_to(status);
              else images[selection.back()].value_string().move_to(status);
#endif // #if cimg_display==0
            } else arg_error("select");
            is_released = false; ++position; continue;
          }

          // Serialize.
          if (!std::strcmp("-serialize",command)) {
#define gmic_serialize(value_type,svalue_type)  \
  if (!std::strcmp(argx,svalue_type)) \
    CImgList<value_type>(g_list,cimg::type<T>::string()==cimg::type<value_type>::string()). \
      get_serialize((bool)is_compressed).move_to(serialized);

            gmic_substitute_args();
            bool is_compressed0 = false;
#ifdef cimg_use_zlib
            is_compressed0 = true;
#endif
            unsigned int is_compressed = is_compressed0?1U:0U, is_gmz = 1;
            if ((cimg_sscanf(argument,"%255[a-z ]%c",
                             argx,&end)==1 ||
                 cimg_sscanf(argument,"%255[a-z ],%u%c",
                             argx,&is_compressed,&end)==2 ||
                 cimg_sscanf(argument,"%255[a-z ],%u,%u%c",
                             argx,&is_compressed,&is_gmz,&end)==3) &&
                (!std::strcmp(argx,"uchar") || !std::strcmp(argx,"unsigned char") ||
                 !std::strcmp(argx,"char") || !std::strcmp(argx,"ushort") ||
                 !std::strcmp(argx,"unsigned short") || !std::strcmp(argx,"short") ||
                 !std::strcmp(argx,"uint") || !std::strcmp(argx,"unsigned int") ||
                 !std::strcmp(argx,"int") || !std::strcmp(argx,"ulong") ||
                 !std::strcmp(argx,"unsigned long") || !std::strcmp(argx,"long") ||
                 !std::strcmp(argx,"float") || !std::strcmp(argx,"double")) &&
                is_compressed<=1 && is_gmz<=1) ++position;
            else { std::strcpy(argx,cimg::type<T>::string()); is_compressed = is_compressed0?1U:0U; is_gmz = 1; }

            print(images,0,
                  "Serialize %simage%s viewed as %sbuffer%s of %s values into a single %scompressed image.",
                  is_gmz?"names ":"",
                  gmic_selection.data(),
                  selection.height()>1?"":"a ",
                  selection.height()>1?"s":"",
                  argx,
                  is_compressed?"":"un");
            if (selection) {
              g_list.assign(selection.height());
              CImgList<unsigned char> gmz_info;
              if (is_gmz) {
                gmz_info.assign(1 + selection.height());
                CImg<char>::string("GMZ").move_to(gmz_info[0]);
              }
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                g_list[l].assign(images[uind],images[uind]?true:false);
                if (is_gmz) CImg<char>::string(images_names[uind]).move_to(gmz_info[1 + l]);
              }
              if (is_gmz) (gmz_info>'x').unroll('y').move_to(g_list);

              CImg<T> serialized;
              gmic_serialize(unsigned char,"uchar")
              else gmic_serialize(unsigned char,"unsigned char")
                else gmic_serialize(char,"char")
                  else gmic_serialize(unsigned short,"ushort")
                    else gmic_serialize(unsigned short,"unsigned short")
                      else gmic_serialize(short,"short")
                        else gmic_serialize(unsigned int,"uint")
                          else gmic_serialize(unsigned int,"unsigned int")
                            else gmic_serialize(int,"int")
                              else gmic_serialize(unsigned long,"ulong")
                                else gmic_serialize(unsigned long,"unsigned long")
                                  else gmic_serialize(long,"long")
                                    else gmic_serialize(float,"float")
                                      else gmic_serialize(double,"double")
                                        else error(images,0,0,
                                                   "Command '-serialize': Invalid "
                                                   "specified pixel type '%s'.",
                                                   argx);
              if (is_double_hyphen) {
                serialized.move_to(images);
                images_names[selection[0]].get_copymark().move_to(images_names);
              } else {
                remove_images(images,images_names,selection,1,selection.height() - 1);
                serialized.move_to(images[selection[0]].assign());
              }
              g_list.assign();
            }
            is_released = false; continue;
          }

        } // command1=='s'.

        //----------------------------
        // Commands starting by '-t..'
        //----------------------------
        else if (command1=='t') {

          // Threshold.
          if (!std::strcmp("-threshold",command)) {
            gmic_substitute_args();
            unsigned int is_soft = 0;
            value = 0;
            sep = 0;
            if ((cimg_sscanf(argument,"%lf%c",
                             &value,&end)==1 ||
                 (cimg_sscanf(argument,"%lf%c%c",
                              &value,&sep,&end)==2 && sep=='%') ||
                 cimg_sscanf(argument,"%lf,%u%c",
                             &value,&is_soft,&end)==2 ||
                 (cimg_sscanf(argument,"%lf%c,%u%c",
                              &value,&sep,&is_soft,&end)==3 && sep=='%')) &&
                is_soft<=1) {
              print(images,0,"%s-threshold image%s by %g%s.",
                    is_soft?"Soft":"Hard",
                    gmic_selection.data(),
                    value,sep=='%'?"%":"");
              cimg_forY(selection,l) {
                CImg<T>& img = gmic_check(images[selection[l]]);
                nvalue = value;
                if (sep=='%' && img) {
                  vmax = (double)img.max_min(vmin);
                  nvalue = vmin + (vmax - vmin)*value/100;
                }
                gmic_apply(threshold((T)nvalue,(bool)is_soft));
              }
              ++position;
            } else {
              g_img.assign(1,selection.height()).fill(0);
#if cimg_display==0
              print(images,0,
                    "Threshold image%s in interactive mode (skipped, no display support).",
                    gmic_selection.data());
#else // #if cimg_display==0
              bool is_available_display = false;
              try {
                is_available_display = (bool)CImgDisplay::screen_width();
              } catch (CImgDisplayException&) {
                print(images,0,
                      "Threshold image%s in interactive mode (skipped, no display available).",
                      gmic_selection.data());
              }
              if (is_available_display) {
                print(images,0,"Threshold image%s in interactive mode.",
                      gmic_selection.data());
                CImgDisplay _disp, &disp = _display_window[0]?_display_window[0]:_disp;
                cimg_forY(selection,l) {
                  CImg<T>& img = gmic_check(images[selection[l]]);
                  value = 0;
                  if (img) {
                    CImg<T> visu = img.depth()>1?img.get_projections2d(img.width()/2,
                                                                       img.height()/2,
                                                                       img.depth()/2).
                      channels(0,cimg::min(3,img.spectrum()) - 1):
                      img.get_channels(0,cimg::min(3,img.spectrum() - 1));
                    const unsigned int
                      w = CImgDisplay::_fitscreen(visu.width(),visu.height(),1,256,-85,false),
                      h = CImgDisplay::_fitscreen(visu.width(),visu.height(),1,256,-85,true);
                    if (disp) disp.resize(w,h,false); else disp.assign(w,h,0,0);
                    double percent = 50;
                    bool stopflag = false, is_clicked = false;
                    int omx = -1, omy = -1;
                    g_img_uc.assign();
                    vmax = (double)img.max_min(vmin);
                    for (disp.show().flush(); !stopflag; ) {
                      static const unsigned char white[] = { 255,255,255 }, black[] = { 0,0,0 };
                      const unsigned int key = disp.key();
                      if (!g_img_uc) {
                        value = vmin + percent*(vmax - vmin)/100;
                        disp.display(((g_img_uc=visu.get_threshold((T)value)*=255).resize(disp)).
                                     draw_text(0,0,"Threshold %g = %.3g%%",
                                               white,black,0.7f,13,value,percent)).
                          set_title("%s (%dx%dx%dx%d)",
                                    basename(images_names[selection[l]].data()),
                                    img.width(),img.height(),img.depth(),img.spectrum()).wait();
                      }
                      const int mx = disp.mouse_x(), my = disp.mouse_y();
                      if (disp.button()) {
                        if (mx>=0 && my>=0 && (mx!=omx || my!=omy)) {
                          percent = (my - 16)*100.0/(disp.height() - 32);
                          if (percent<0) percent = 0; else if (percent>101) percent = 101;
                          omx = mx; omy = my; g_img_uc.assign();
                        }
                        is_clicked = true;
                      } else if (is_clicked) break;
                      if (disp.is_closed() || (key && key!=cimg::keyCTRLLEFT)) stopflag = true;
                      if (key==cimg::keyD && disp.is_keyCTRLLEFT()) {
                        disp.resize(cimg_fitscreen(3*disp.width()/2,3*disp.height()/2,1),
                                    stopflag=false).set_key(cimg::keyD,false);
                        g_img_uc.assign();
                      }
                      if (key==cimg::keyC && disp.is_keyCTRLLEFT()) {
                        disp.resize(cimg_fitscreen(2*disp.width()/3,2*disp.height()/3,1),
                                    stopflag=false).set_key(cimg::keyC,false);
                        g_img_uc.assign();
                      }
                      if (disp.is_resized()) { disp.resize(false); g_img_uc.assign(); }
                    }
                    value = vmin + percent*(vmax - vmin)/100;
                    print(images,0,
                          "Hard-threshold image [%d] by %g = %.3g%%.",
                          selection[l],value,percent);
                    gmic_apply(threshold((T)value));
                    cimg_snprintf(title,_title.width(),"%g",value);
                    CImg<char>::string(title).move_to(status);
                  } else gmic_apply(replace(img));
                  g_img[l] = (T)value;
                }
                g_img_uc.assign();
              }
#endif // #if cimg_display==0
              g_img.value_string().move_to(status);
              g_img.assign();
            }
            is_released = false; continue;
          }

          // Tangent.
          gmic_simple_command("-tan",tan,"Compute pointwise tangent of image%s.");

          // Draw text.
          if (!std::strcmp("-text",command)) {
            gmic_substitute_args();
            name.assign(4096);
            *argx = *argy = *argz = *name = *color = 0;
            float x = 0, y = 0, height = 13;
            sep = sepx = sepy = 0;
            opacity = 1;
            if ((cimg_sscanf(argument,"%4095[^,]%c",
                             name.data(),&end)==1 ||
                 cimg_sscanf(argument,"%4095[^,],%255[0-9.eE%+-]%c",
                             name.data(),argx,&end)==2 ||
                 cimg_sscanf(argument,"%4095[^,],%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             name.data(),argx,argy,&end)==3 ||
                 cimg_sscanf(argument,"%4095[^,],%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             name.data(),argx,argy,argz,&end)==4 ||
                 cimg_sscanf(argument,"%4095[^,],%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],%f%c",
                             name.data(),argx,argy,argz,&opacity,&end)==5 ||
                 cimg_sscanf(argument,"%4095[^,],%255[0-9.eE%+-],%255[0-9.eE%+-],%255[0-9.eE%+-],%f,"
                             "%4095[0-9.eEinfa,+-]%c",
                             name.data(),argx,argy,argz,&opacity,color,&end)==6) &&
                (!*argx ||
                 cimg_sscanf(argx,"%f%c",&x,&end)==1 ||
                 (cimg_sscanf(argx,"%f%c%c",&x,&sepx,&end)==2 && sepx=='%')) &&
                (!*argy ||
                 cimg_sscanf(argy,"%f%c",&y,&end)==1 ||
                 (cimg_sscanf(argy,"%f%c%c",&y,&sepy,&end)==2 && sepy=='%')) &&
                (!*argz ||
                 cimg_sscanf(argz,"%f%c",&height,&end)==1 ||
                 (cimg_sscanf(argz,"%f%c%c",&height,&sep,&end)==2 && sep=='%')) &&
                height>=0) {
              strreplace_fw(name);
              print(images,0,"Draw text '%s' at position (%g%s,%g%s) on image%s, with font "
                    "height %g%s, opacity %g and color (%s).",
                    name.data(),
                    x,sepx=='%'?"%":"",
                    y,sepy=='%'?"%":"",
                    gmic_selection.data(),
                    height,sep=='%'?"%":"",
                    opacity,
                    *color?color:"default");
              cimg::strunescape(name);
              unsigned int nb_cols = 1;
              for (const char *s = color; *s; ++s) if (*s==',') ++nb_cols;
              cimg_forY(selection,l) {
                CImg<T> &img = images[selection[l]];
                const unsigned int font_height = (unsigned int)cimg::round(sep=='%'?
                                                                           height*img.height()/100:height);
                g_img.assign(cimg::max(img.spectrum(),(int)nb_cols),1,1,1,0).fill(color,true,false);
                const int
                  nx = (int)cimg::round(sepx=='%'?x*(img.width() - 1)/100:x),
                  ny = (int)cimg::round(sepy=='%'?y*(img.height() - 1)/100:y);
                gmic_apply(gmic_draw_text(nx,ny,name,g_img,0,opacity,font_height,nb_cols));
              }
            } else arg_error("text");
            g_img.assign();
            is_released = false; ++position; continue;
          }

          // Texturize 3d object.
          if (!std::strcmp("-texturize3d",command)) {
            gmic_substitute_args();
            CImg<unsigned int> ind_texture, ind_coords;
            sep = *argx = *argy = 0;
            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                              argx,&sep,&end)==2 && sep==']') ||
                 (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],[%255[a-zA-Z0-9_.%+-]%c%c",
                              argx,argy,&sep,&end)==3 && sep==']')) &&
                (ind_texture=selection2cimg(argx,images.size(),images_names,"-texturize3d",true,
                                            false,CImg<char>::empty())).height()==1 &&
                (!*argy || (ind_coords=selection2cimg(argy,images.size(),images_names,
                                                      "-texturize3d",true,
                                                      false,CImg<char>::empty())).height()==1)) {
              if (ind_coords)
                print(images,0,
                      "Texturize 3d object%s with texture [%u] and texture coordinates [%u].",
                      gmic_selection.data(),*ind_texture,*ind_coords);
              else
                print(images,0,"Texturize 3d object%s with texture [%u].",
                      gmic_selection.data(),*ind_texture);
              const CImg<T>
                texture = gmic_image_arg(*ind_texture),
                coords = ind_coords?gmic_image_arg(*ind_coords):CImg<T>();
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                CImg<T>& img = images[uind];
                try { gmic_apply(texturize_CImg3d(texture,coords)); }
                catch (CImgException&) {
                  if (!img.is_CImg3d(true,&(*message=0)))
                    error(images,0,0,
                          "Command '-texturize3d': Invalid 3d object [%d], "
                          "in selected image%s (%s).",
                          uind,gmic_selection_err.data(),message.data());
                  else throw;
                }
              }
            } else arg_error("texturize3d");
            is_released = false; ++position; continue;
          }

          // Tridiagonal solve.
          if (!std::strcmp("-trisolve",command)) {
            gmic_substitute_args();
            sep = 0;
            if (cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep,&end)==2 &&
                sep==']' &&
                (ind=selection2cimg(indices,images.size(),images_names,"-trisolve",true,
                                    false,CImg<char>::empty())).height()==1) {
              print(images,0,"Solve tridiagonal system AX = B, with B-vector%s and tridiagonal "
                    "A-matrix [%d].",
                    gmic_selection.data(),*ind);
              const CImg<T> A = gmic_image_arg(*ind);
              cimg_forY(selection,l) gmic_apply(solve_tridiagonal(A));
            } else arg_error("trisolve");
            is_released = false; ++position; continue;
          }

          // Hyperbolic tangent.
          gmic_simple_command("-tanh",tanh,"Compute pointwise hyperbolic tangent of image%s.");

        } // command1=='t'.

        //----------------------------
        // Commands starting by '-u..'
        //----------------------------
        else if (command1=='u') {

          // Unroll.
          if (!std::strcmp("-unroll",command)) {
            gmic_substitute_args();
            axis = 'y';
            if ((*argument=='x' || *argument=='y' ||
                 *argument=='z' || *argument=='c') && !argument[1]) {
              axis = *argument;
              ++position;
            }
            print(images,0,"Unroll image%s along the '%c'-axis.",
                  gmic_selection.data(),
                  axis);
            cimg_forY(selection,l) gmic_apply(unroll(axis));
            is_released = false; continue;
          }

          // Remove custom command.
          if (!std::strcmp("-uncommand",item)) {
            gmic_substitute_args();
            if (argument[0]=='*' && !argument[1]) { // Discard all custom commands.
              unsigned int nb_commands = 0;
              for (unsigned int i = 0; i<512; ++i) {
                nb_commands+=commands[i].size();
                commands[i].assign();
                commands_names[i].assign();
                commands_has_arguments[i].assign();
              }
              print(images,0,"Discard definitions of all custom commmands (%u command%s discarded).",
                    nb_commands,nb_commands>1?"s":"");
            } else { // Discard one or several custom command.
              g_list_c = CImg<char>::string(argument).get_split(CImg<char>::vector(','),0,false);
              print(images,0,"Discard last definition of custom command%s '%s'",
                    g_list_c.width()>1?"s":"",
                    gmic_argument_text_printed());
              unsigned int nb_removed = 0;
              cimglist_for(g_list_c,l) {
                CImg<char>& arg_command = g_list_c[l];
                arg_command.resize(1,arg_command.height() + 1,1,1,0);
                strreplace_fw(arg_command);
                if (*arg_command) {
                  const int ind = (int)hashcode(arg_command,false);
                  cimglist_for(commands_names[ind],l)
                    if (!std::strcmp(commands_names[ind][l],arg_command)) {
                      commands_names[ind].remove(l);
                      commands[ind].remove(l);
                      commands_has_arguments[ind].remove(l);
                      ++nb_removed; break;
                    }
                }
              }
              if (is_verbose) {
                cimg::mutex(29);
                unsigned int siz = 0;
                for (unsigned int l = 0; l<512; ++l) siz+=commands[l].size();
                std::fprintf(cimg::output()," (%u found, %u command%s left).",
                             nb_removed,siz,siz>1?"s":"");
                std::fflush(cimg::output());
                cimg::mutex(29,0);
              }
              g_list_c.assign();
            }
            ++position; continue;
          }

          // Unserialize.
          if (!std::strcmp("-unserialize",command)) {
            print(images,0,"Unserialize image%s.",
                  gmic_selection.data());
            int off = 0;
            cimg_forY(selection,l) {
              const unsigned int uind = selection[l] + off;
              const CImg<T>& img = gmic_check(images[uind]);
              g_list = CImgList<T>::get_unserialize(img);
              if (g_list) {
                const CImg<T>& back = g_list.back();
                if (back.width()==1 && back.depth()==1 && back.spectrum()==1 &&
                    back[0]=='G' && back[1]=='M' && back[2]=='Z' && !back[3]) { // .gmz serialization.
                  g_list_c = back.get_split(CImg<char>::vector(0),0,false);
                  g_list_c.remove(0);
                  cimglist_for(g_list_c,l)
                    g_list_c[l].resize(1,g_list_c[l].height() + 1,1,1,0).unroll('x');
                  if (g_list_c) g_list.remove();
                } else { // .cimg[z] serialization.
                  g_list_c.insert(images_names[uind]);
                  g_list_c.insert(g_list.width() - 1,images_names[uind].get_copymark());
                }
                if (g_list_c.width()>g_list.width())
                  g_list_c.remove(g_list.width(),g_list_c.width() - 1);
                else if (g_list_c.width()<g_list.width())
                  g_list_c.insert(g_list.width() - g_list_c.width(),CImg<char>::string("[unnamed]"));
                if (is_double_hyphen) {
                  g_list.move_to(images,~0U);
                  g_list_c.move_to(images_names,~0U);
                } else {
                  images.remove(uind); images_names.remove(uind);
                  off+=(int)g_list.size() - 1;
                  g_list.move_to(images,uind);
                  g_list_c.move_to(images_names,uind);
                }
              }
            }
            g_list.assign(); g_list_c.assign();
            is_released = false; continue;
          }

        } // command1=='u'.

        //----------------------------
        // Commands starting by '-v..'
        //----------------------------
        else if (command1=='v') {

          // Set verbosity
          // (actually only display a log message, since it has been already processed before).
          if (is_verbose_command) {
            if (*argument=='-' && !argument[1])
              print(images,0,"Decrement verbosity level (set to %d).",
                    verbosity);
            else if (*argument=='+' && !argument[1]) {
              if (is_very_verbose) print(images,0,"Increment verbosity level (set to %d).",
                                         verbosity);
            } else if ((verbosity>=0 && old_verbosity>=0) || is_debug)
              print(images,0,"Set verbosity level to %d.",
                    verbosity);
            if (is_verbose_argument) ++position;
            continue;
          }

          // Vanvliet filter.
          if (!std::strcmp("-vanvliet",command)) {
            gmic_substitute_args();
            unsigned int order = 0;
            float sigma = 0;
            axis = sep = 0;
            boundary = 1;
            if ((cimg_sscanf(argument,"%f,%u,%c%c",&sigma,&order,&axis,&end)==3 ||
                 (cimg_sscanf(argument,"%f%c,%u,%c%c",&sigma,&sep,&order,&axis,&end)==4 &&
                  sep=='%') ||
                 cimg_sscanf(argument,"%f,%u,%c,%u%c",&sigma,&order,&axis,&boundary,&end)==4 ||
                 (cimg_sscanf(argument,"%f%c,%u,%c,%u%c",
                              &sigma,&sep,&order,&axis,&boundary,&end)==5 && sep=='%')) &&
                sigma>=0 && order<=3 && (axis=='x' || axis=='y' || axis=='z' || axis=='c') &&
                boundary<=1) {
              print(images,0,"Apply %u-order Vanvliet filter on image%s, along axis '%c' with standard "
                    "deviation %g%s and %s boundary conditions.",
                    order,gmic_selection.data(),axis,
                    sigma,sep=='%'?"%":"",
                    boundary?"neumann":"dirichlet");
              if (sep=='%') sigma = -sigma;
              cimg_forY(selection,l) gmic_apply(vanvliet(sigma,order,axis,(bool)boundary));
            } else arg_error("vanvliet");
            is_released = false; ++position; continue;
          }

        } // command1=='v'.

        //----------------------------
        // Commands starting by '-w..'
        //----------------------------
        else if (command1=='w') {

          // While.
          if (!std::strcmp("-while",item)) {
            gmic_substitute_args();
            const CImg<char>& s = callstack.back();
            if (s[0]!='*' || s[1]!='d')
              error(images,0,0,
                    "Command '-while': Not associated to a '-do' command within the same scope.");
            float _is_cond = 0;
            bool is_filename = false;
            if (cimg_sscanf(argument,"%f%c",&_is_cond,&end)!=1) {
              is_filename = true;
              name.assign(argument,(unsigned int)std::strlen(argument) + 1);
              strreplace_fw(name);
              _is_cond = (float)check_filename(name);
            }
            const bool is_cond = (bool)_is_cond;
            if (is_very_verbose) print(images,0,"Reach '-while' command -> %s '%s' %s.",
                                       is_filename?"file":"boolean",
                                       gmic_argument_text_printed(),
                                       is_filename?(is_cond?"exists":
                                                    "does not exist"):
                                       (is_cond?"is true":"is false"));
            if (is_cond) {
              position = dowhiles.back()(0);
              next_debug_line = debug_line; next_debug_filename = debug_filename;
              continue;
            } else {
              if (is_very_verbose) print(images,0,"End 'do..while' block.");
              dowhiles.remove();
              callstack.remove();
            }
            ++position; continue;
          }

          // Warning.
          if (!is_double_hyphen && !std::strcmp("-warn",command)) {
            gmic_substitute_args();
            bool force_visible = false;
            if ((*argument=='0' || *argument=='1') && argument[1]==',') {
              force_visible = *argument=='1'; argument+=2;
            }
            name.assign(argument,(unsigned int)std::strlen(argument) + 1);
            cimg::strunescape(name);
            if (is_restriction) warn(images,&selection,force_visible,"%s",name.data());
            else warn(images,0,force_visible,"%s",name.data());
            ++position; continue;
          }

          // Display images in display window.
          unsigned int wind = 0;
          if (!is_double_hyphen &&
              (!std::strcmp("-window",command) ||
               cimg_sscanf(command,"-window%u%c",&wind,&end)==1 ||
               cimg_sscanf(command,"-w%u%c",&wind,&end)==1) &&
              wind<10) {
            gmic_substitute_args();
            int norm = -1, fullscreen = -1;
            float dimw = -1, dimh = -1, posx = -1, posy = -1;
            sep0 = sep1 = sepx = sepy = *argx = *argy = *argz = *argc = *title = 0;
            if ((cimg_sscanf(argument,"%255[0-9.eE%+-]%c",
                             argx,&end)==1 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-]%c",
                             argx,argy,&end)==2 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%d%c",
                             argx,argy,&norm,&end)==3 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%d,%d%c",
                             argx,argy,&norm,&fullscreen,&end)==4 ||
                 cimg_sscanf(argument,
                             "%255[0-9.eE%+-],%255[0-9.eE%+-],%d,%d,%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-]%c",
                             argx,argy,&norm,&fullscreen,argz,argc,&end)==6 ||
                 cimg_sscanf(argument,
                             "%255[0-9.eE%+-],%255[0-9.eE%+-],%d,%d,%255[0-9.eE%+-],"
                             "%255[0-9.eE%+-],%255[^\n]",
                             argx,argy,&norm,&fullscreen,argz,argc,title)==7 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%d,%d,%255[^\n]",
                             &(*argx=*argz=*argc=0),argy,&norm,&fullscreen,title)==5 ||
                 cimg_sscanf(argument,"%255[0-9.eE%+-],%255[0-9.eE%+-],%d,%255[^\n]",
                             argx,argy,&(norm=fullscreen=-1),title)==4 ||
                 (norm=fullscreen=-1,cimg_sscanf(argument,
                                                 "%255[0-9.eE%+-],%255[0-9.eE%+-],%255[^\n]",
                                                 argx,argy,title))==3) &&
                (cimg_sscanf(argx,"%f%c",&dimw,&end)==1 ||
                 (cimg_sscanf(argx,"%f%c%c",&dimw,&sep0,&end)==2 && sep0=='%')) &&
                (!*argy ||
                 cimg_sscanf(argy,"%f%c",&dimh,&end)==1 ||
                 (cimg_sscanf(argy,"%f%c%c",&dimh,&sep1,&end)==2 && sep1=='%')) &&
                (!*argz ||
                 cimg_sscanf(argz,"%f%c",&posx,&end)==1 ||
                 (cimg_sscanf(argz,"%f%c%c",&posx,&sepx,&end)==2 && sepx=='%')) &&
                (!*argc ||
                 cimg_sscanf(argc,"%f%c",&posy,&end)==1 ||
                 (cimg_sscanf(argc,"%f%c%c",&posy,&sepy,&end)==2 && sepy=='%')) &&
                (dimw>=0 || dimw==-1) &&
                (dimh>=0 || dimh==-1) &&
                norm>=-1 && norm<=3) ++position;
            else {
              dimw = dimh = -1;
              norm = fullscreen = -1;
              posx = posy = -1;
              sep0 = sep1 = 0;
            }
            if (dimh==0) dimw = 0;
            strreplace_fw(title);

#if cimg_display==0
            print(images,0,
                  "Display image%s in display window [%d] (skipped, no display support).",
                  gmic_selection.data(),
                  wind);
#else // #if cimg_display==0

            // Get images to display and compute associated optimal size.
            unsigned int optw = 0, opth = 0;
            if (dimw && dimh) cimg_forY(selection,l) {
                const CImg<T>& img = gmic_check(images[selection[l]]);
                if (img) {
                  g_list.insert(img,~0U,true);
                  optw+=img._width + (img._depth>1?img._depth:0U);
                  if (img.height()>(int)opth) opth = img._height + (img._depth>1?img._depth:0U);
                }
              }
            optw = optw?optw:sep0=='%'?CImgDisplay::screen_width():256;
            opth = opth?opth:sep1=='%'?CImgDisplay::screen_height():256;
            dimw = dimw<0?-1:cimg::round(sep0=='%'?optw*dimw/100:dimw);
            dimh = dimh<0?-1:cimg::round(sep1=='%'?opth*dimh/100:dimh);

            const bool is_move = posx!=-1 || posy!=-1;
            bool is_available_display = false;
            try {
              is_available_display = (bool)CImgDisplay::screen_width();
            } catch (CImgDisplayException&) {
              print(images,0,
                    "Display image%s in display window [%d] (skipped, no display available).",
                    gmic_selection.data(),
                    wind);
            }
            if (is_available_display) {
              CImgDisplay &disp = _display_window[wind];

              if (!dimw || !dimh) { // Close.
                print(images,0,"Close display window [%d].",
                      wind);
                disp.assign();
              } else {
                if (disp) { // Update.
                  if (!selection) disp.show();
                  disp.resize(dimw>0?(int)dimw:disp.window_width(),
                              dimh>0?(int)dimh:disp.window_height(),
                              false);
                  if (is_move) {
                    if (sepx=='%') posx*=(CImgDisplay::screen_width() - disp.window_width())/100.0f;
                    if (sepy=='%') posy*=(CImgDisplay::screen_height() - disp.window_height())/100.0f;
                    disp.move((int)posx,(int)posy);
                  }
                  if (norm>=0) disp._normalization = (unsigned int)norm;
                  if (*title && std::strcmp(disp.title(),title)) disp.set_title("%s",title);
                  if (fullscreen>=0 && (bool)fullscreen!=disp.is_fullscreen()) disp.toggle_fullscreen(false);
                } else { // Create.
                  if (!*title) cimg_snprintf(title,_title.width(),"[G'MIC] Window #%u",wind);
                  disp.assign(dimw>0?(int)dimw:optw,
                              dimh>0?(int)dimh:opth,
                              title,norm<0?3:norm,
                              fullscreen<0?false:(bool)fullscreen,
                              is_move);
                  if (is_move) {
                    if (sepx=='%') posx*=(CImgDisplay::screen_width() - disp.window_width())/100.0f;
                    if (sepy=='%') posy*=(CImgDisplay::screen_height() - disp.window_height())/100.0f;
                    disp.move((int)posx,(int)posy);
                  }
                  if (norm==2) {
                    if (g_list) disp._max = (float)g_list.max_min(disp._min);
                    else { disp._min = 0; disp._max = 255; }
                  }
                }
                if (is_move) print(images,0,
                                   "Display image%s in %dx%d %sdisplay window [%d], "
                                   "with%snormalization, "
                                   "%sfullscreen, at position (%s,%s) and title '%s'.",
                                   gmic_selection.data(),
                                   disp.width(),disp.height(),disp.is_fullscreen()?"fullscreen ":"",
                                   wind,
                                   disp.normalization()==0?"out ":disp.normalization()==1?" ":
                                   disp.normalization()==2?" 1st-time ":" auto-",
                                   disp.is_fullscreen()?"":"no ",
                                   argz,argc,
                                   disp.title());
                else print(images,0,
                           "Display image%s in %dx%d %sdisplay window [%d], with%snormalization, "
                           "%sfullscreen and title '%s'.",
                           gmic_selection.data(),
                           disp.width(),disp.height(),disp.is_fullscreen()?"fullscreen ":"",
                           wind,
                           disp.normalization()==0?"out ":disp.normalization()==1?" ":
                           disp.normalization()==2?" 1st-time ":" auto-",
                           disp.is_fullscreen()?"":"no ",
                           disp.title());
                if (g_list) g_list.display(disp);
              }
            }
            g_list.assign();

#endif // #if cimg_display==0
            is_released = true; continue;
          }

          // Warp.
          if (!std::strcmp("-warp",command)) {
            gmic_substitute_args();
            unsigned int interpolation = 1, mode = 0;
            float nb_frames = 1;
            boundary = 1;
            sep = 0;
            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",
                              indices,&sep,&end)==2 && sep==']')||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u%c",
                             indices,&mode,&end)==2 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u,%u%c",
                             indices,&mode,&interpolation,&end)==3 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u,%u,%u%c",
                             indices,&mode,&interpolation,&boundary,&end)==4 ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u,%u,%u,%f%c",
                             indices,&mode,&interpolation,&boundary,&nb_frames,&end)==5) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-warp",true,
                                    false,CImg<char>::empty())).height()==1 &&
                mode<=3 && interpolation<=2 && boundary<=2 && nb_frames>=0.5) {
              const CImg<T> warping_field = gmic_image_arg(*ind);
              nb_frames = cimg::round(nb_frames);
              if (nb_frames==1) {
                print(images,0,"Warp image%s with %s-%s displacement field [%u], %s interpolation, "
                      "%s boundary conditions.",
                      gmic_selection.data(),
                      mode<=2?"backward":"forward",mode%2?"relative":"absolute",
                      *ind,
                      interpolation==2?"cubic":interpolation==1?"linear":"nearest-neighbor",
                      boundary==0?"dirichlet":boundary==1?"neumann":"periodic");
                cimg_forY(selection,l) gmic_apply(warp(warping_field,mode,interpolation,boundary));
              } else {
                print(images,0,"Warp image%s with %s-%s displacement field [%u], %s interpolation, "
                      "%s boundary conditions and %d frames.",
                      gmic_selection.data(),
                      mode<=2?"backward":"forward",mode%2?"relative":"absolute",
                      *ind,
                      interpolation==2?"cubic":interpolation==1?"linear":"nearest-neighbor",
                      boundary==0?"dirichlet":boundary==1?"neumann":"periodic",
                      (int)nb_frames);
                unsigned int off = 0;
                cimg_forY(selection,l) {
                  const unsigned int _ind = selection[l] + off;
                  CImg<T>& img = gmic_check(images[_ind]);
                  g_list.assign((int)nb_frames);
                  name = images_names[_ind];
                  cimglist_for(g_list,t)
                    g_list[t] = img.get_warp(warping_field*((t + 1.0f)/nb_frames),mode,
                                             interpolation,boundary);
                  if (is_double_hyphen) {
                    images_names.insert((int)nb_frames,name.copymark());
                    g_list.move_to(images,~0U);
                  } else {
                    off+=(int)nb_frames - 1;
                    images_names.insert((int)nb_frames - 1,name.get_copymark(),_ind);
                    images.remove(_ind); g_list.move_to(images,_ind);
                  }
                }
                g_list.assign();
              }
            } else arg_error("warp");
            is_released = false; ++position; continue;
          }

          // Watershed transform.
          if (!std::strcmp("-watershed",command)) {
            gmic_substitute_args();
            unsigned int is_filled = 1;
            sep = 0;
            if (((cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sep,&end)==2 &&
                  sep==']') ||
                 cimg_sscanf(argument,"[%255[a-zA-Z0-9_.%+-]],%u%c",
                             indices,&is_filled,&end)==2) &&
                (ind=selection2cimg(indices,images.size(),images_names,"-watershed",true,
                                    false,CImg<char>::empty())).height()==1 &&
                is_filled<=1) {
              print(images,0,"Compute watershed transform of image%s with priority map [%u] and "
                    "%sfilling.",
                    gmic_selection.data(),*ind,is_filled?"":"no ");
              const CImg<T> priority = gmic_image_arg(*ind);
              cimg_forY(selection,l) gmic_apply(watershed(priority,(bool)is_filled));
            } else arg_error("watershed");
            is_released = false; ++position; continue;
          }

          // Wait for a given delay of for user events on display window.
          if (!is_double_hyphen && !std::strcmp("-wait",command)) {
            gmic_substitute_args();
            if (!is_restriction)
              CImg<unsigned int>::vector(0,1,2,3,4,5,6,7,8,9).move_to(selection);
            float delay = 0;
            if (cimg_sscanf(argument,"%f%c",
                            &delay,&end)==1) ++position;
            else delay = 0;
#if cimg_display==0
            if (!delay)
              print(images,0,
                    "Wait for user events on display window%s (skipped, no display support).",
                    gmic_selection.data());
            else {
              delay = cimg::round(delay);
              print(images,0,
                    "%s for %g milliseconds according to display window%s.",
                    delay<0?"Sleep":"Wait",delay,
                    gmic_selection.data());
              if (delay<0) cimg::sleep((unsigned int)-delay);
              else cimg::wait((unsigned int)delay);
            }
#else // #if cimg_display==0
            if (!delay) {
              print(images,0,"Wait for user events on display window%s.",
                    gmic_selection.data());
              CImgDisplay *const iw = _display_window;
              switch (selection.height()) {
              case 1 : CImgDisplay::wait(iw[selection[0]]); break;
              case 2 : CImgDisplay::wait(iw[selection[0]],iw[selection[1]]); break;
              case 3 : CImgDisplay::wait(iw[selection[0]],iw[selection[1]],iw[selection[2]]);
                break;
              case 4 : CImgDisplay::wait(iw[selection[0]],iw[selection[1]],iw[selection[2]],
                                         iw[selection[3]]);
                break;
              case 5 : CImgDisplay::wait(iw[selection[0]],iw[selection[1]],iw[selection[2]],
                                         iw[selection[3]],iw[selection[4]]);
                break;
              case 6 : CImgDisplay::wait(iw[selection[0]],iw[selection[1]],iw[selection[2]],
                                         iw[selection[3]],iw[selection[4]],iw[selection[5]]);
                break;
              case 7 : CImgDisplay::wait(iw[selection[0]],iw[selection[1]],iw[selection[2]],
                                         iw[selection[3]],iw[selection[4]],iw[selection[5]],
                                         iw[selection[6]]);
                break;
              case 8 : CImgDisplay::wait(iw[selection[0]],iw[selection[1]],iw[selection[2]],
                                         iw[selection[3]],iw[selection[4]],iw[selection[5]],
                                         iw[selection[6]],iw[selection[7]]);
                break;
              case 9 : CImgDisplay::wait(iw[selection[0]],iw[selection[1]],iw[selection[2]],
                                         iw[selection[3]],iw[selection[4]],iw[selection[5]],
                                         iw[selection[6]],iw[selection[7]],iw[selection[8]]);
                break;
              case 10 : CImgDisplay::wait(iw[selection[0]],iw[selection[1]],iw[selection[2]],
                                          iw[selection[3]],iw[selection[4]],iw[selection[5]],
                                          iw[selection[6]],iw[selection[7]],iw[selection[8]],
                                          iw[selection[9]]);
                break;
              }
            } else if (delay<0) {
              delay = cimg::round(-delay);
              print(images,0,
                    "Flush display events of display window%s and wait for %g milliseconds.",
                    gmic_selection.data(),delay);
              cimg_forY(selection,l) _display_window[selection[l]].flush();
              if (selection && _display_window[selection[0]])
                _display_window[selection[0]].wait((unsigned int)delay);
              else cimg::wait((unsigned int)delay);
            } else {
              delay = cimg::round(delay);
              print(images,0,"Wait for %g milliseconds according to display window%s.",
                    delay,
                    gmic_selection.data());
              if (selection && _display_window[selection[0]])
                _display_window[selection[0]].wait((unsigned int)delay);
              else cimg::sleep((unsigned int)delay);
            }
#endif // #if cimg_display==0
            continue;
          }

        } // command1=='w'.

        //----------------------------
        // Commands starting by '-x..'
        //----------------------------
        else if (command1=='x') {

          // Bitwise xor.
          gmic_arithmetic_command("-xor",
                                  operator^=,
                                  "Compute bitwise XOR of image%s by %g%s",
                                  gmic_selection.data(),value,ssep,Tlong,
                                  operator^=,
                                  "Compute bitwise XOR of image%s by image [%d]",
                                  gmic_selection.data(),ind[0],
                                  "Compute bitwise XOR of image%s by expression %s",
                                  gmic_selection.data(),gmic_argument_text_printed(),
                                  "Compute sequential bitwise XOR of image%s");

        } // command1=='x'.

        //----------------------------
        // Other (special) commands.
        //----------------------------

        // If..[elif]..[else]..endif.
        if (!std::strcmp("-if",item) || (!std::strcmp("-elif",item) && check_elif)) {
          gmic_substitute_args();
          check_elif = false;
          float _is_cond = 0;
          bool is_filename = false;
          if (cimg_sscanf(argument,"%f%c",&_is_cond,&end)!=1) {
            is_filename = true;
            name.assign(argument,(unsigned int)std::strlen(argument) + 1);
            strreplace_fw(name);
            _is_cond = (float)check_filename(name);
          }
          const bool is_cond = (bool)_is_cond;
          if (item[1]=='i') {
            if (is_debug_info && debug_line!=~0U) {
              cimg_snprintf(argx,_argx.width(),"*if#%u",debug_line);
              CImg<char>::string(argx).move_to(callstack);
            } else CImg<char>::string("*if").move_to(callstack);
            if (is_very_verbose) print(images,0,"Start '-if..-endif' block -> %s '%s' %s.",
                                       is_filename?"file":"boolean",
                                       gmic_argument_text_printed(),
                                       is_filename?(is_cond?"exists":"does not exist"):
                                       (is_cond?"is true":"is false"));
          } else if (is_very_verbose) print(images,0,"Reach '-elif' block -> %s '%s' %s.",
                                            is_filename?"file":"boolean",
                                            gmic_argument_text_printed(),
                                            is_filename?(is_cond?"exists":
                                                         "does not exist"):
                                            (is_cond?"is true":"is false"));
          if (!is_cond) {
            for (int nb_ifs = 1; nb_ifs && position<commands_line.size(); ++position) {
              const char *const it = commands_line[position].data();
              if (*it==1 &&
                  cimg_sscanf(commands_line[position].data() + 1,"%x,%x",&_debug_line,&(_debug_filename=0))>0) {
                is_debug_info = true; next_debug_line = _debug_line; next_debug_filename = _debug_filename;
              } else if (!std::strcmp("-if",it)) ++nb_ifs;
              else if (!std::strcmp("-endif",it)) { --nb_ifs; if (!nb_ifs) --position; }
              else if (nb_ifs==1) {
                if (!std::strcmp("-else",it)) --nb_ifs;
                else if (!std::strcmp("-elif",it)) { --nb_ifs; check_elif = true; --position;}
              }
            }
            continue;
          }
          ++position; continue;
        }

        // Break and continue.
        bool is_continue = false;
        if (!std::strcmp("-break",item) ||
            (!std::strcmp("-continue",item) && (is_continue=true)==true)) {
          const char
	    *const com = is_continue?"continue":"break",
	    *const Com = is_continue?"Continue":"Break";
          unsigned int callstack_repeat = 0, callstack_do = 0, callstack_local = 0;
          for (unsigned int l = callstack.size() - 1; l; --l) {
            const char *const s = callstack[l].data();
            if (s[0]=='*' && s[1]=='r') { callstack_repeat = l; break; }
            else if (s[0]=='*' && s[1]=='d') { callstack_do = l; break; }
            else if (s[0]=='*' && s[1]=='l') { callstack_local = l; break; }
            else if (s[0]!='*' || s[1]!='i') break;
          }
          const char *stb = 0, *ste = 0;
          unsigned int callstack_ind = 0;
          int level = 0;
          if (callstack_repeat) {
            print(images,0,"%s %scurrent 'repeat..done' block.",
                  Com,is_continue?"to next iteration of ":"");
            for (level = 1; level && position<commands_line.size(); ++position) {
              const char *it = commands_line[position].data();
              if (!std::strcmp("-repeat",it)) ++level;
              else if (!std::strcmp("-done",it)) --level;
            }
            callstack_ind = callstack_repeat;
            stb = "repeat"; ste = "done";
          } else if (callstack_do) {
            print(images,0,"%s %scurrent 'do..while' block.",
                  Com,is_continue?"to next iteration of ":"");
            for (level = 1; level && position<commands_line.size(); ++position) {
              const char *it = commands_line[position].data();
              if (!std::strcmp("-do",it)) ++level;
              else if (!std::strcmp("-while",it)) --level;
            }
            callstack_ind = callstack_do;
            stb = "do"; ste = "while";
          } else if (callstack_local) {
            print(images,0,"%s %scurrent local environment.",
                  Com,is_continue?"to end of ":"");
            for (level = 1; level && position<commands_line.size(); ++position) {
              const char *it = commands_line[position].data();
              if (!std::strcmp("-local",it) || !std::strcmp("-l",it) ||
                  !std::strcmp("--local",it) || !std::strcmp("--l",it) ||
                  !std::strncmp("-local[",it,7) || !std::strncmp("-l[",it,3) ||
                  !std::strncmp("--local[",it,8) || !std::strncmp("--l[",it,4)) ++level;
              else if (!std::strcmp("-endlocal",it) || !std::strcmp("-endl",it)) --level;
            }
            callstack_ind = callstack_local;
            stb = "local"; ste = "endlocal";
          } else {
            print(images,0,"%s",Com);
            error(images,0,0,
                  "Command '-%s': There are no loops or local environment to %s.",com,com);
            continue;
          }
          if (level && position>=commands_line.size())
            error(images,0,0,
                  "Command '-%s': Missing associated '-%s' command.",stb,ste);
          if (is_continue || callstack_local) {
	    if (callstack_ind<callstack.size() - 1) callstack.remove(callstack_ind + 1,callstack.size() - 1);
	    --position;
	  } else {
            callstack.remove(callstack_ind,callstack.size() - 1);
            if (callstack_do) { dowhiles.remove(); ++position; } else repeatdones.remove();
          }
          continue;
        }

        // Quit.
        if (!std::strcmp("-quit",item)) {
          print(images,0,"Quit G'MIC interpreter.");
          dowhiles.assign();
          repeatdones.assign();
          position = commands_line.size();
          is_released = is_quit = true;
          *is_abort = true;
          break;
        }

        // Compute direct or inverse FFT.
        const bool inv_fft = !std::strcmp("-ifft",command);
        if (!std::strcmp("-fft",command) || inv_fft) {
          gmic_substitute_args();
          bool is_valid_argument = *argument!=0;
          if (is_valid_argument) for (const char *s = argument; *s; ++s) {
              const char _s = *s;
              if (_s!='x' && _s!='y' && _s!='z') { is_valid_argument = false; break; }
            }
          if (is_valid_argument) {
            print(images,0,"Compute %sfourier transform of image%s along the '%s'-ax%cs with complex pair%s",
                  inv_fft?"inverse ":"",
                  gmic_selection.data(),
                  gmic_argument_text_printed(),
                  std::strlen(argument)>1?'e':'i',
                  selection.height()>2?"s":selection.height()>=1?"":"().");
            ++position;
          } else
            print(images,0,"Compute %sfourier transform of image%s with complex pair%s",
                  inv_fft?"inverse ":"",
                  gmic_selection.data(),
                  selection.height()>2?"s":selection.height()>=1?"":" ().");
          cimg_forY(selection,l) {
            const unsigned int
	      uind0 = selection[l],
	      uind1 = l + 1<selection.height()?selection[l + 1]:~0U;
            CImg<T> &img0 = gmic_check(images[uind0]),
                    &img1 = uind1!=~0U?gmic_check(images[uind1]):CImg<T>::empty();
            name = images_names[uind0];
            if (uind1!=~0U) { // Complex transform.
              if (is_verbose) {
                cimg::mutex(29);
                std::fprintf(cimg::output()," ([%u],[%u])%c",uind0,uind1,
			     l>=selection.height() - 2?'.':',');
                std::fflush(cimg::output());
                cimg::mutex(29,0);
              }
              if (is_double_hyphen) {
                g_list.assign(img0,img1);
                if (is_valid_argument) for (const char *s = argument; *s; ++s) g_list.FFT(*s,inv_fft);
                else g_list.FFT(inv_fft);
                g_list.move_to(images,~0U);
                images_names.insert(2,name.copymark());
              } else {
                g_list.assign(2);
                g_list[0].swap(img0);
                g_list[1].swap(img1);
                if (is_valid_argument) for (const char *s = argument; *s; ++s) g_list.FFT(*s,inv_fft);
                else g_list.FFT(inv_fft);
                g_list[0].swap(img0);
                g_list[1].swap(img1);
                name.get_copymark().move_to(images_names[uind1]);
                name.move_to(images_names[uind0]);
              }
              ++l;
            } else { // Real transform.
              if (is_verbose) {
                cimg::mutex(29);
                std::fprintf(cimg::output()," ([%u],0)%c",uind0,
			     l>=selection.height() - 2?'.':',');
                std::fflush(cimg::output());
                cimg::mutex(29,0);
              }
              if (is_double_hyphen) {
                g_list.assign(img0);
                CImg<T>(g_list[0].width(),g_list[0].height(),g_list[0].depth(),g_list[0].spectrum(),0).
                  move_to(g_list);
                if (is_valid_argument) for (const char *s = argument; *s; ++s) g_list.FFT(*s,inv_fft);
                else g_list.FFT(inv_fft);
                g_list.move_to(images,~0U);
                images_names.insert(2,name.copymark());
              } else {
                g_list.assign(1);
                g_list[0].swap(img0);
                CImg<T>(g_list[0].width(),g_list[0].height(),g_list[0].depth(),g_list[0].spectrum(),0).
                  move_to(g_list);
                if (is_valid_argument) for (const char *s = argument; *s; ++s) g_list.FFT(*s,inv_fft);
                else g_list.FFT(inv_fft);
                g_list[0].swap(img0);
                g_list[1].move_to(images,uind0 + 1);
                name.get_copymark().move_to(images_names,uind0 + 1);
                name.move_to(images_names[uind0]);
              }
            }
          }
          g_list.assign();
          is_released = false; continue;
        }

        // Inverse scale of a 3d object.
        const bool divide3d = !std::strcmp("-div3d",command);
        if (!std::strcmp("-mul3d",command) || divide3d) {
          gmic_substitute_args();
          float sx = 0, sy = 1, sz = 1;
          if ((cimg_sscanf(argument,"%f%c",
                           &sx,&end)==1 && ((sz=sy=sx),1)) ||
              cimg_sscanf(argument,"%f,%f%c",
                          &sx,&sy,&end)==2 ||
              cimg_sscanf(argument,"%f,%f,%f%c",
                          &sx,&sy,&sz,&end)==3) {
            if (divide3d)
              print(images,0,"Scale 3d object%s with factors (1/%g,1/%g,1/%g).",
                    gmic_selection.data(),
                    sx,sy,sz);
            else
              print(images,0,"Scale 3d object%s with factors (%g,%g,%g).",
                    gmic_selection.data(),
                    sx,sy,sz);
            cimg_forY(selection,l) {
              const unsigned int uind = selection[l];
              CImg<T>& img = images[uind];
              try {
                if (divide3d) { gmic_apply(scale_CImg3d(1/sx,1/sy,1/sz)); }
                else gmic_apply(scale_CImg3d(sx,sy,sz));
              } catch (CImgException&) {
                if (!img.is_CImg3d(true,&(*message=0)))
                  error(images,0,0,
                        "Command '-%s3d': Invalid 3d object [%d], in selected image%s (%s).",
                        divide3d?"div":"mul",uind,gmic_selection_err.data(),message.data());
                else throw;
              }
            }
          } else { if (divide3d) arg_error("div3d"); else arg_error("mul3d"); }
          is_released = false; ++position; continue;
        }

        // Check for a custom command, and execute it, if found.
        if (!is_input_command) {
          const char *custom_command = 0, cc = *(command + 1);
          bool custom_command_found = false, has_arguments = false, _is_noarg = false;
          CImg<char> substituted_command;
          if ((cc>='a' && cc<='z') || (cc>='A' && cc<='Z') || cc=='_') {
            const int ind = (int)hashcode(command + 1,false);
            cimglist_for(commands_names[ind],l) {
              custom_command = commands_names[ind][l].data();
              const char
                *const command_code = commands[ind][l].data(),
                *const command_code_back = &commands[ind][l].back();

              if (!std::strcmp(command + 1,custom_command)) {
                custom_command_found = true;
                if (is_debug) {
                  CImg<char> command_code_text(264);
                  const unsigned int ls = (unsigned int)std::strlen(command_code);
                  if (ls>=264) {
                    std::memcpy(command_code_text.data(),command_code,128);
                    std::memcpy(command_code_text.data() + 128,"(...)",5);
                    std::memcpy(command_code_text.data() + 133,command_code + ls - 130,131);
                  } else std::strcpy(command_code_text.data(),command_code);
                  for (char *ptrs = command_code_text, *ptrd = ptrs; *ptrs || (bool)(*ptrd=0);
                       ++ptrs)
                    if (*ptrs==1) while (*ptrs!=' ') ++ptrs; else *(ptrd++) = *ptrs;
                  debug(images,"Found custom command '%s: %s' (%s).",
                        custom_command,command_code_text.data(),
                        commands_has_arguments[ind](l,0)?"takes arguments":"takes no arguments");
                }

                CImgList<char> arguments(32);
                // Set $0 to be the command name.
                CImg<char>::string(custom_command).move_to(arguments[0]);
                unsigned int nb_arguments = 0;

                if (commands_has_arguments[ind](l,0)) { // Command takes arguments.
                  gmic_substitute_args();

                  // Extract possible command arguments.
                  for (const char *ss = argument, *_ss = ss; _ss; ss =_ss + 1)
                    if ((_ss=std::strchr(ss,','))!=0) {
                      if (ss==_ss) ++nb_arguments;
                      else {
                        if (++nb_arguments>=arguments.size())
                          arguments.insert(2 + 2*nb_arguments - arguments.size());
                        CImg<char> arg_item(ss,(unsigned int)(_ss - ss + 1));
                        arg_item.back() = 0;
                        arg_item.move_to(arguments[nb_arguments]);
                      }
                    } else {
                      if (*ss) {
                        if (++nb_arguments>=arguments.size())
                          arguments.insert(1 + nb_arguments - arguments.size());
                        if (*ss!=',') CImg<char>::string(ss).move_to(arguments[nb_arguments]);
                      }
                      break;
                    }

                  if (is_debug) {
                    debug(images,"Found %d given argument%s for command '%s'%s",
                          nb_arguments,nb_arguments!=1?"s":"",
                          custom_command,nb_arguments>0?":":".");
                    for (unsigned int i = 1; i<=nb_arguments; ++i)
                      if (arguments[i]) debug(images,"  $%d = '%s'",i,arguments[i].data());
                      else debug(images,"  $%d = (undefined)",i);
                  }
                }

                // Substitute arguments in custom command expression.
                CImgList<char> substituted_items;
                CImg<char> inbraces;

                for (const char *nsource = command_code; *nsource;)
                  if (*nsource!='$') {

                    // If not starting with '$'.
                    const char *const nsource0 = nsource;
                    nsource = std::strchr(nsource0,'$');
                    if (!nsource) nsource = command_code_back;
                    CImg<char>(nsource0,(unsigned int)(nsource - nsource0)).move_to(substituted_items);
                  } else { // '$' expression found.
                    CImg<char> substr(324);
                    inbraces.assign(1,1,1,1,0);
                    int ind = 0, ind1 = 0, l_inbraces = 0;
                    bool is_braces = false;
                    sep = 0;

                    if (nsource[1]=='{') {
                      const char *const ptr_beg = nsource + 2, *ptr_end = ptr_beg;
                      unsigned int p = 0;
                      for (p = 1; p>0 && *ptr_end; ++ptr_end) {
                        if (*ptr_end=='{') ++p;
                        if (*ptr_end=='}') --p;
                      }
                      if (p) { CImg<char>(nsource++,1).move_to(substituted_items); continue; }
                      l_inbraces = (int)(ptr_end - ptr_beg - 1);
                      if (l_inbraces>0) inbraces.assign(ptr_beg,l_inbraces + 1).back() = 0;
                      is_braces = true;
                    }

                    // Substitute $# -> maximum indice of known arguments.
                    if (nsource[1]=='#') {
                      nsource+=2;
                      cimg_snprintf(substr,substr.width(),"%u",nb_arguments);
                      CImg<char>(substr.data(),(unsigned int)std::strlen(substr)).move_to(substituted_items);
                      has_arguments = true;

                      // Substitute $* -> copy of the specified arguments string.
                    } else if (nsource[1]=='*') {
                      nsource+=2;
                      CImg<char>(argument,(unsigned int)std::strlen(argument)).move_to(substituted_items);
                      has_arguments = true;

                      // Substitute $"*" -> copy of the specified "quoted" arguments string.
                    } else if (nsource[1]=='\"' && nsource[2]=='*' && nsource[3]=='\"') {
                      nsource+=4;
                      for (unsigned int i = 1; i<=nb_arguments; ++i) {
                        CImg<char>(1,1,1,1,'\"').move_to(substituted_items);
                        CImg<char>(arguments[i].data(),arguments[i].width() - 1).
                          move_to(substituted_items);
                        if (i==nb_arguments) CImg<char>(1,1,1,1,'\"').move_to(substituted_items);
                        else CImg<char>(2,1,1,1,'\"',',').move_to(substituted_items);
                      }
                      has_arguments = true;

                      // Substitute $= -> transfer (quoted) arguments to named variables.
                    } else if (nsource[1]=='=' &&
                               cimg_sscanf(nsource + 2,"%255[a-zA-Z0-9_]",title)==1 &&
                               (*title<'0' || *title>'9')) {
                      nsource+=2 + std::strlen(title);
                      for (unsigned int i = 0; i<=nb_arguments; ++i) {
                        cimg_snprintf(substr,substr.width()," %s%u=\"",title,i);
                        CImg<char>(substr.data(),(unsigned int)std::strlen(substr)).move_to(substituted_items);
                        CImg<char>(arguments[i].data(),arguments[i].width() - 1).
                          move_to(substituted_items);
                        CImg<char>(2,1,1,1,'\"',' ').move_to(substituted_items);
                      }
                      has_arguments = true;

                      // Substitute $i and ${i} -> value of the i^th argument.
                    } else if ((cimg_sscanf(nsource,"$%d",&ind)==1 ||
                                (cimg_sscanf(nsource,"${%d%c",&ind,&sep)==2 && sep=='}'))) {
                      const int nind = ind + (ind<0?(int)nb_arguments + 1:0);
                      if ((nind<=0 && ind) || nind>=arguments.width() || !arguments[nind]) {
                        error(images,0,custom_command,
                              "Command '-%s': Undefined argument '$%d', in expression '$%s%d%s' "
                              "(for %u argument%s specified).",
                              custom_command,ind,sep=='}'?"{":"",ind,sep=='}'?"}":"",
                              nb_arguments,nb_arguments!=1?"s":"");
                      }
                      nsource+=cimg_snprintf(substr,substr.width(),"$%d",ind) + (sep=='}'?2:0);
                      if (arguments[nind].width()>1)
                        CImg<char>(arguments[nind].data(),arguments[nind].width() - 1).
                          move_to(substituted_items);
                      if (nind!=0) has_arguments = true;

                      // Substitute ${i=$j} -> value of the i^th argument, or the default value,
                      // i.e. the value of another argument.
                    } else if (cimg_sscanf(nsource,"${%d=$%d%c",&ind,&ind1,&sep)==3 && sep=='}' &&
                               ind>0) {
                      const int nind1 = ind1 + (ind1<0?(int)nb_arguments + 1:0);
                      if (nind1<=0 || nind1>=arguments.width() || !arguments[nind1])
                        error(images,0,custom_command,
                              "Command '-%s': Undefined argument '$%d', in expression '${%d=$%d}' "
                              "(for %u argument%s specified).",
                              custom_command,ind1,ind,ind1,
                              nb_arguments,nb_arguments!=1?"s":"");
                      nsource+=cimg_snprintf(substr,substr.width(),"${%d=$%d}",ind,ind1);
                      if (ind>=arguments.width()) arguments.insert(2 + 2*ind - arguments.size());
                      if (!arguments[ind]) {
                        arguments[ind] = arguments[nind1];
                        if (ind>(int)nb_arguments) nb_arguments = (unsigned int)ind;
                      }
                      if (arguments[ind].width()>1)
                        CImg<char>(arguments[ind].data(),arguments[ind].width() - 1).
                          move_to(substituted_items);
                      has_arguments = true;

                      // Substitute ${i=$#} -> value of the i^th argument, or the default value,
                      // i.e. the maximum indice of known arguments.
                    } else if (cimg_sscanf(nsource,"${%d=$#%c",&ind,&sep)==2 && sep=='}' &&
                               ind>0) {
                      if (ind>=arguments.width()) arguments.insert(2 + 2*ind - arguments.size());
                      if (!arguments[ind]) {
                        cimg_snprintf(substr,substr.width(),"%u",nb_arguments);
                        CImg<char>::string(substr).move_to(arguments[ind]);
                        if (ind>(int)nb_arguments) nb_arguments = (unsigned int)ind;
                      }
                      nsource+=cimg_snprintf(substr,substr.width(),"${%d=$#}",ind);
                      if (arguments[ind].width()>1)
                        CImg<char>(arguments[ind].data(),arguments[ind].width() - 1).
                          move_to(substituted_items);
                      has_arguments = true;

                      // Substitute ${i=default} -> value of the i^th argument,
                      // or the specified default value.
                    } else if (cimg_sscanf(inbraces,"%d%c",&ind,&sep)==2 && sep=='=' &&
                               ind>0) {
                      nsource+=l_inbraces + 3;
                      if (ind>=arguments.width()) arguments.insert(2 + 2*ind - arguments.size());
                      if (!arguments[ind]) {
                        CImg<char>::string(inbraces.data() +
                                           cimg_snprintf(substr,substr.width(),"%d=",ind)).
                          move_to(arguments[ind]);
                        if (ind>(int)nb_arguments) nb_arguments = (unsigned int)ind;
                      }
                      if (arguments[ind].width()>1)
                        CImg<char>(arguments[ind].data(),arguments[ind].width() - 1).
                          move_to(substituted_items);
                      has_arguments = true;

                      // Substitute any other expression starting by '$'.
                    } else {

                      // Substitute ${subset} -> values of the selected subset of arguments,
                      // separated by ','.
                      if (is_braces) {
                        if ((*inbraces>='a' && *inbraces<='z') ||
                            (*inbraces>='A' && *inbraces<='Z') ||
                            *inbraces=='_' || !*inbraces ||
                            std::strchr(inbraces,' ')) {
                          CImg<char>(nsource++,1).move_to(substituted_items);
                        } else {
                          CImg<unsigned int> inds;
                          const int _verbosity = verbosity;
                          const bool _is_debug = is_debug;
                          bool is_valid_subset = true;
                          verbosity = -16384; is_debug = false;
                          CImg<char> _status;
                          status.move_to(_status); // Save status because 'selection2cimg' can change it.
                          try {
                            inds = selection2cimg(inbraces,nb_arguments + 1,
                                                  CImgList<char>::empty(),"",false,
                                                  false,CImg<char>::empty());
                          } catch (...) { inds.assign(); is_valid_subset = false; }
                          _status.move_to(status);
                          verbosity = _verbosity; is_debug = _is_debug;
                          if (is_valid_subset) {
                            nsource+=l_inbraces + 3;
                            if (inds) {
                              cimg_forY(inds,j) {
                                const unsigned int uind = inds[j];
                                if (uind) has_arguments = true;
                                if (!arguments[uind])
                                  error(images,0,custom_command,
                                        "Command '-%s': Undefined argument '$%d', "
                                        "in expression '${%s}'.",
                                        custom_command,uind,inbraces.data());
                                substituted_items.insert(arguments[uind]);
                                substituted_items.back().back() = ',';
                              }
                              if (substituted_items.back().width()>1)
                                --(substituted_items.back()._width);
                              else substituted_items.remove();
                              has_arguments = true;
                            }
                          } else CImg<char>(nsource++,1).move_to(substituted_items);
                        }
                      } else CImg<char>(nsource++,1).move_to(substituted_items);
                    }
                  }
                CImg<char>::vector(0).move_to(substituted_items);
                (substituted_items>'x').move_to(substituted_command);

                // Substitute special character codes appearing outside strings.
                bool is_dquoted = false;
                for (char *s = substituted_command.data(); *s; ++s) {
                  const char c = *s;
                  if (c=='\"') is_dquoted = !is_dquoted;
                  if (!is_dquoted) *s = c<' '?(c==gmic_dollar?'$':c==gmic_lbrace?'{':c==gmic_rbrace?'}':
                                               c==gmic_comma?',':c==gmic_dquote?'\"':c):c;
                }

                if (is_debug) {
                  CImg<char> command_code_text(264);
                  const unsigned int l = (unsigned int)std::strlen(substituted_command.data());
                  if (l>=264) {
                    std::memcpy(command_code_text.data(),substituted_command.data(),128);
                    std::memcpy(command_code_text.data() + 128,"(...)",5);
                    std::memcpy(command_code_text.data() + 133,substituted_command.data() + l - 130,131);
                  } else std::strcpy(command_code_text.data(),substituted_command.data());
                  for (char *ptrs = command_code_text, *ptrd = ptrs; *ptrs || (bool)(*ptrd=0);
                       ++ptrs)
                    if (*ptrs==1) while (*ptrs!=' ') ++ptrs; else *(ptrd++) = *ptrs;
                  debug(images,"Expand command line for command '%s' to: '%s'.",
                        custom_command,command_code_text.data());
                }
                break;
              }
            }
          }

          if (custom_command_found) {
            const CImgList<char>
	      ncommands_line = commands_line_to_CImgList(substituted_command.data());
            CImg<unsigned int> nvariables_sizes(512);
	    cimg_forX(nvariables_sizes,l) nvariables_sizes[l] = variables[l]->size();
            g_list.assign(selection.height());
            g_list_c.assign(selection.height());

            unsigned int nposition = 0;
            gmic_exception exception;
            const unsigned int
              previous_debug_filename = debug_filename,
              previous_debug_line = debug_line;
            CImg<char>::string(custom_command).move_to(callstack);
            if (is_double_hyphen) {
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                g_list[l] = images[uind];
                g_list_c[l] = images_names[uind];
              }
              try {
                is_debug_info = false;
                _run(ncommands_line,nposition,g_list,g_list_c,images,images_names,nvariables_sizes,&_is_noarg,
                     argument,&selection);
              } catch (gmic_exception &e) {
                cimg::swap(exception._command_help,e._command_help);
                cimg::swap(exception._message,e._message);
              }
              g_list.move_to(images,~0U);
              cimglist_for(g_list_c,l) g_list_c[l].copymark();
              g_list_c.move_to(images_names,~0U);
            } else {
              cimg::mutex(27);
              cimg_forY(selection,l) {
                const unsigned int uind = selection[l];
                if ((images[uind]._width || images[uind]._height) && !images[uind]._spectrum) {
                  selection2string(selection,images_names,1,name);
                  error(images,0,0,
                        "Command '-%s': Invalid selection%s "
                        "(image [%u] is already used in another thread).",
                        custom_command,name.data() + (*name=='s'?1:0),uind);
                }
                if (images[uind].is_shared())
                  g_list[l].assign(images[uind],false);
                else {
                  g_list[l].swap(images[uind]);
                  // Small hack to be able to track images of the selection passed to the new environment.
                  std::memcpy(&images[uind]._width,&g_list[l]._data,sizeof(void*));
                  images[uind]._spectrum = 0;
                }
                g_list_c[l] = images_names[uind];   // Make a copy to be still able to recognize '-pass[label]'.
              }
              cimg::mutex(27,0);

              try {
                is_debug_info = false;
                _run(ncommands_line,nposition,g_list,g_list_c,images,images_names,nvariables_sizes,&_is_noarg,
                     argument,&selection);
              } catch (gmic_exception &e) {
                cimg::swap(exception._command_help,e._command_help);
                cimg::swap(exception._message,e._message);
              }

              const unsigned int nb = cimg::min((unsigned int)selection.height(),g_list.size());
              if (nb>0) {
                for (unsigned int i = 0; i<nb; ++i) {
                  const unsigned int uind = selection[i];
                  if (images[uind].is_shared()) {
                    images[uind] = g_list[i];
                    g_list[i].assign();
                  } else images[uind].swap(g_list[i]);
                  images_names[uind].swap(g_list_c[i]);
                }
                g_list.remove(0,nb - 1);
                g_list_c.remove(0,nb - 1);
              }
              if (nb<(unsigned int)selection.height())
                remove_images(images,images_names,selection,nb,selection.height() - 1);
              else if (g_list) {
                const unsigned int uind0 = selection?selection.back() + 1:images.size();
                g_list_c.move_to(images_names,uind0);
                g_list.move_to(images,uind0);
              }
            }
	    for (unsigned int l = 0; l<nvariables_sizes._width - 2; ++l) if (variables[l]->size()>nvariables_sizes[l]) {
		variables_names[l]->remove(nvariables_sizes[l],variables[l]->size() - 1);
		variables[l]->remove(nvariables_sizes[l],variables[l]->size() - 1);
	      }
            callstack.remove();
            debug_filename = previous_debug_filename;
            debug_line = previous_debug_line;
            is_return = false;
            g_list.assign();
            g_list_c.assign();
            if (has_arguments && !_is_noarg) ++position;
            if (exception._message) throw exception;
            continue;
          }
        }
      }  // if (*item=='-') {

      // Variable assignment.
      if (!is_input_command) {
        const char *const s_eq = std::strchr(item,'='), *new_value = 0;
        if (s_eq) {
          sep0 = s_eq>item?*(s_eq - 1):0;
          sep1 = s_eq>item + 1?*(s_eq - 2):0;
          if (cimg_sscanf(item,"%255[a-zA-Z0-9_]",title)==1 && (*title<'0' || *title>'9')) {
            pattern = (unsigned int)std::strlen(title);
            if ((sep0=='<' || sep0=='>') && sep1==sep0 && s_eq==item + pattern + 2) {
              new_value = set_variable(title,s_eq + 1,sep0,variables_sizes);
              print(images,0,"Update %s variable %s%c%c='%s' -> %s='%s'.",
                    *title=='_'?"global":"local",
                    title,sep0,sep0,s_eq + 1,title,new_value);
              continue;
            } else if ((sep0=='+' || sep0=='-' || sep0=='*' || sep0=='/' ||
                        sep0=='%' || sep0=='&' || sep0=='|' || sep0=='^') && s_eq==item + pattern + 1) {
              new_value = set_variable(title,s_eq + 1,sep0,variables_sizes);
              print(images,0,"Update %s variable %s%c='%s' -> %s='%s'.",
                    *title=='_'?"global":"local",
                    title,sep0,s_eq + 1,title,new_value);
              continue;
            } else if (s_eq==item + pattern) {
              set_variable(title,s_eq + 1,'=',variables_sizes);
              print(images,0,"Set %s variable %s='%s'.",
                    *title=='_'?"global":"local",
                    title,s_eq + 1);
              continue;
            }
          }
        }
      }

      // Input.
      if (is_input_command) ++position; else { std::strcpy(command,"-input"); argument = item; *restriction = 0; }
      gmic_substitute_args();
      if (!is_restriction || !selection) selection.assign(1,1,1,1,images.size());

      CImg<char> indicesy(256), indicesz(256), indicesc(256);
      float dx = 0, dy = 1, dz = 1, dc = 1, nb = 1;
      CImg<unsigned int> indx, indy, indz, indc;
      CImgList<char> input_images_names;
      CImgList<T> input_images;
      sepx = sepy = sepz = sepc = *indices = *indicesy = *indicesz = *indicesc = *argx = *argy = *argz = *argc = 0;

      CImg<char> arg_input(argument,(unsigned int)std::strlen(argument) + 1);
      strreplace_fw(arg_input);

      CImg<char> _gmic_selection;
      if (is_verbose) selection2string(selection,images_names,0,_gmic_selection);

      if (*arg_input=='0' && !arg_input[1]) {

        // Empty image.
        print(images,0,"Input empty image at position%s",
              _gmic_selection.data());
        input_images.assign(1);
        CImg<char>::string("[empty]").move_to(input_images_names);


      } else if ((cimg_sscanf(arg_input,"[%255[a-zA-Z_0-9%.eE%^,:+-]%c%c",indices,&sep,&end)==2 &&
                  sep==']') ||
                 cimg_sscanf(arg_input,"[%255[a-zA-Z_0-9%.eE%^,:+-]]x%f%c",indices,&nb,&end)==2) {

        // Nb copies of existing images.
        nb = cimg::round(nb);
        const CImg<unsigned int> inds = selection2cimg(indices,images.size(),images_names,
						       "-input",true,false,CImg<char>::empty());
        CImg<char> s_tmp;
        if (is_verbose) selection2string(inds,images_names,1,s_tmp);
        if (nb<=0) arg_error("input");
        if (nb!=1)
          print(images,0,"Input %u copies of image%s at position%s",
                (unsigned int)nb,
                s_tmp.data(),
                _gmic_selection.data());
        else
          print(images,0,"Input copy of image%s at position%s",
                s_tmp.data(),
                _gmic_selection.data());
        for (unsigned int i = 0; i<(unsigned int)nb; ++i) cimg_foroff(inds,l) {
            input_images.insert(gmic_check(images[inds[l]]));
            input_images_names.insert(images_names[inds[l]].get_copymark());
          }
      } else if ((sep=0,true) &&
                 (cimg_sscanf(arg_input,"%255[][a-zA-Z0-9_.eE%+-]%c",
                              argx,&end)==1 ||
                  cimg_sscanf(arg_input,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-]%c",
                              argx,argy,&end)==2 ||
                  cimg_sscanf(arg_input,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],"
			      "%255[][a-zA-Z0-9_.eE%+-]%c",
                              argx,argy,argz,&end)==3 ||
                  cimg_sscanf(arg_input,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],"
			      "%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-]%c",
                              argx,argy,argz,argc,&end)==4 ||
                  cimg_sscanf(arg_input,"%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],"
			      "%255[][a-zA-Z0-9_.eE%+-],%255[][a-zA-Z0-9_.eE%+-],%c",
                              argx,argy,argz,argc,&sep)==5) &&
                 ((cimg_sscanf(argx,"[%255[a-zA-Z0-9_.%+-]%c%c",indices,&sepx,&end)==2 &&
		   sepx==']' &&
                   (indx=selection2cimg(indices,images.size(),images_names,"-input",true,
                                        false,CImg<char>::empty())).height()==1) ||
                  (cimg_sscanf(argx,"%f%c",&dx,&end)==1 && dx>=1) ||
                  (cimg_sscanf(argx,"%f%c%c",&dx,&sepx,&end)==2 && dx>0 && sepx=='%')) &&
                 (!*argy ||
                  (cimg_sscanf(argy,"[%255[a-zA-Z0-9_.%+-]%c%c",indicesy.data(),&sepy,&end)==2 &&
		   sepy==']' &&
                   (indy=selection2cimg(indicesy,images.size(),images_names,"-input",true,
                                        false,CImg<char>::empty())).height()==1) ||
                  (cimg_sscanf(argy,"%f%c",&dy,&end)==1 && dy>=1) ||
                  (cimg_sscanf(argy,"%f%c%c",&dy,&sepy,&end)==2 && dy>0 && sepy=='%')) &&
                 (!*argz ||
                  (cimg_sscanf(argz,"[%255[a-zA-Z0-9_.%+-]%c%c",indicesz.data(),&sepz,&end)==2 &&
		   sepz==']' &&
                   (indz=selection2cimg(indicesz,images.size(),images_names,"-input",true,
                                        false,CImg<char>::empty())).height()==1) ||
                  (cimg_sscanf(argz,"%f%c",&dz,&end)==1 && dz>=1) ||
                  (cimg_sscanf(argz,"%f%c%c",&dz,&sepz,&end)==2 && dz>0 && sepz=='%')) &&
                 (!*argc ||
                  (cimg_sscanf(argc,"[%255[a-zA-Z0-9_.%+-]%c%c",indicesc.data(),&sepc,&end)==2 &&
		   sepc==']' &&
                   (indc=selection2cimg(indicesc,images.size(),images_names,"-input",true,
                                        false,CImg<char>::empty())).height()==1) ||
                  (cimg_sscanf(argc,"%f%c",&dc,&end)==1 && dc>=1) ||
                  (cimg_sscanf(argc,"%f%c%c",&dc,&sepc,&end)==2 && dc>0 && sepc=='%'))) {

        // New image with specified dimensions and optionally values.
        if (indx) { dx = (float)gmic_check(images[*indx]).width(); sepx = 0; }
        if (indy) { dy = (float)gmic_check(images[*indy]).height(); sepy = 0; }
        if (indz) { dz = (float)gmic_check(images[*indz]).depth(); sepz = 0; }
        if (indc) { dc = (float)gmic_check(images[*indc]).spectrum(); sepc = 0; }
        int idx = 0, idy = 0, idz = 0, idc = 0;
        const CImg<T>& img = images.size()?gmic_check(images.back()):CImg<T>::empty();
        if (sepx=='%') { idx = (int)cimg::round(dx*img.width()/100); if (!idx) ++idx; }
        else idx = (int)cimg::round(dx);
        if (sepy=='%') { idy = (int)cimg::round(dy*img.height()/100); if (!idy) ++idy; }
        else idy = (int)cimg::round(dy);
        if (sepz=='%') { idz = (int)cimg::round(dz*img.depth()/100); if (!idz) ++idz; }
        else idz = (int)cimg::round(dz);
        if (sepc=='%') { idc = (int)cimg::round(dc*img.spectrum()/100); if (!idc) ++idc; }
        else idc = (int)cimg::round(dc);
        if (idx<=0 || idy<=0 || idz<=0 || idc<=0) arg_error("input");
        CImg<char> s_values;
        if (sep) {
          const char *_s_values = arg_input.data() + std::strlen(argx) + std::strlen(argy) +
            std::strlen(argz) + std::strlen(argc) + 4;
          s_values.assign(_s_values,(unsigned int)std::strlen(_s_values) + 1);
          cimg::strpare(s_values,'\'',true,false);
          strreplace_fw(s_values);
          CImg<char> s_values_text(72);
          const unsigned int l = (unsigned int)std::strlen(s_values);
          if (l>=72) {
            std::memcpy(s_values_text.data(),s_values.data(),32);
            std::memcpy(s_values_text.data() + 32,"(...)",5);
            std::memcpy(s_values_text.data() + 37,s_values.data() + l - 34,35);  // Last '\0' is included.
          } else std::strcpy(s_values_text,s_values);
          print(images,0,"Input image at position%s, with values '%s'",
                _gmic_selection.data(),s_values_text.data());
        } else
          print(images,0,"Input black image at position%s",
                _gmic_selection.data());
        CImg<T> new_image(idx,idy,idz,idc,0);
        if (s_values) {
          new_image.fill(s_values.data(),true,true,&images,&images);
          cimg_snprintf(title,_title.width(),"[image of '%s']",s_values.data());
          CImg<char>::string(title).move_to(input_images_names);
        } else CImg<char>::string("[unnamed]").move_to(input_images_names);
        new_image.move_to(input_images);

      } else if (*arg_input=='(' && arg_input[std::strlen(arg_input) - 1]==')') {

        // New IxJxKxL image specified as array.
        unsigned int cx = 0, cy = 0, cz = 0, cc = 0, maxcx = 0, maxcy = 0, maxcz = 0;
        const char *nargument = 0;
        for (nargument = arg_input.data() + 1; *nargument; ) {
          CImg<char> s_value(256); *s_value = 0;
          char separator = 0;
          value = 0;
          if (cimg_sscanf(nargument,"%255[0-9.eE+-]%c",s_value.data(),&separator)==2 &&
              cimg_sscanf(s_value,"%lf%c",&value,&end)==1) {
            if (cx>maxcx) maxcx = cx;
            if (cy>maxcy) maxcy = cy;
            if (cz>maxcz) maxcz = cz;
            switch (separator) {
            case '^' : cx = cy = cz = 0; ++cc; break;
            case '/' : cx = cy = 0; ++cz; break;
            case ';' : cx = 0; ++cy; break;
            case ',' : ++cx; break;
            case ')' : break;
            default : arg_error("input");
            }
            nargument+=std::strlen(s_value) + 1;
          } else break;
        }
        if (*nargument) arg_error("input");
        CImg<T> img(maxcx + 1,maxcy + 1,maxcz + 1,cc + 1,0);
        cx = cy = cz = cc = 0;
        for (nargument = arg_input.data() + 1; *nargument; ) {
          CImg<char> s_value(256); *s_value = 0;
          char separator = 0;
          value = 0;
          if (cimg_sscanf(nargument,"%255[0-9.eE+-]%c",s_value.data(),&separator)==2 &&
              cimg_sscanf(s_value,"%lf%c",&value,&end)==1) {
            img(cx,cy,cz,cc) = (T)value;
            switch (separator) {
            case '^' : cx = cy = cz = 0; ++cc; break;
            case '/' : cx = cy = 0; ++cz; break;
            case ';' : cx = 0; ++cy; break;
            default : ++cx;
            }
            nargument+=std::strlen(s_value) + (separator?1:0);
          } else break;
        }
        print(images,0,"Input image at position%s, with values %s",
              _gmic_selection.data(),
              gmic_argument_text_printed());
        img.move_to(input_images);
        arg_input.move_to(input_images_names);

      } else {

        // Input filename.
        char cext[12];
        CImg<char> _filename(4096), filename_tmp(256), options(256);
        *cext = *_filename = *filename_tmp = *options = 0;
        bool is_network_file = false;
        if (cimg_sscanf(argument,"%11[a-zA-Z]:%4095[^,],%255s",
                        cext,_filename.data(),options.data())<2 ||
            !cext[1] || // length of 'ext' must be >=2 (avoid case 'C:\\..' on Windows).
            !cimg::strcasecmp(cext,"http") || !cimg::strcasecmp(cext,"https")) {
          *cext = *_filename = *options = 0;
          if (cimg_sscanf(argument,"%4095[^,],%255s",_filename.data(),options.data())!=2) {
            std::strncpy(_filename,argument,_filename.width() - 1);
            _filename[_filename.width() - 1] = 0;
          }
        }
        strreplace_fw(_filename);
        strreplace_fw(options);
        CImg<char> __filename0 = CImg<char>::string(_filename);
        const char *const _filename0 = __filename0.data();

        // Test for network file requests.
        if (!cimg::strncasecmp(_filename,"http://",7) ||
            !cimg::strncasecmp(_filename,"https://",8)) {
          try {
            cimg::load_network(_filename,filename_tmp);
          } catch (CImgIOException&) {
            print(images,0,"Input file '%s' at position%s",
                  _filename0,
                  _gmic_selection.data());
            error(images,0,0,
                  "Unreachable network file '%s'.",
                  gmic_argument_text());
          }
          is_network_file = true;
          std::strncpy(_filename,filename_tmp,_filename.width() - 1);
          _filename[_filename.width() - 1] = 0;
          *filename_tmp = 0;
        }

        if (*cext) { // Force input to be read as a '.ext' file : generate random filename.
          if (*_filename=='-' && (!_filename[1] || _filename[1]=='.')) {
            // Simplify filename 'ext:-.foo' as '-.ext'.
            cimg_snprintf(_filename,_filename.width(),"-.%s",cext);
            *cext = 0;
          } else {
            std::FILE *file = 0;
            do {
              cimg_snprintf(filename_tmp,filename_tmp.width(),"%s%c%s.%s",
                            cimg::temporary_path(),cimg_file_separator,
                            cimg::filenamerand(),cext);
              if ((file=std::fopen(filename_tmp,"rb"))!=0) std::fclose(file);
            } while (file);

            // Make a temporary copy (or link) of the original file.
#if cimg_OS==1
            const char *const _filename_path = realpath(_filename,0);
            if (symlink(_filename_path,filename_tmp))
              CImg<unsigned char>::get_load_raw(_filename).save_raw(filename_tmp);
            std::free((void*)_filename_path);
#else // #if cimg_OS==1
            CImg<unsigned char>::get_load_raw(_filename).save_raw(filename_tmp);
#endif // #if cimg_OS==1
          }
        }

        const char
          *const filename = *filename_tmp?filename_tmp:_filename,
          *const ext = cimg::split_filename(filename);
        const bool is_stdin = *filename=='-' && (!filename[1] || filename[1]=='.');

        const char *file_type = 0;
        std::FILE *const file = is_stdin?0:std::fopen(filename,"rb");
        long siz = 0;
        if (file) {
          std::fseek(file,0,SEEK_END);
          siz = std::ftell(file);
          std::rewind(file);
          file_type = *ext?0:cimg::ftype(file,0);
          std::fclose(file);
        }
        if (!is_stdin && file && siz==0) { // Empty file -> Insert an empty image.
          input_images_names.insert(__filename0);
          input_images.insert(1);
        } else if (!cimg::strcasecmp("off",ext) || (file_type && !std::strcmp(file_type,"off"))) {

          // 3d object .off file.
          print(images,0,"Input 3d object '%s' at position%s",
                _filename0,_gmic_selection.data());

          if (*options)
            error(images,0,0,
                  "Command '-input': File '%s', format does not take any input options (options '%s' specified).",
                  _filename0,options.data());

          CImg<float>::get_load_off(primitives,g_list_f,filename).move_to(vertices);
          const CImg<float> opacities(1,primitives.size(),1,1,1);
          vertices.object3dtoCImg3d(primitives,g_list_f,opacities,false).move_to(input_images);
          primitives.assign();
          g_list_f.assign();
          input_images_names.insert(__filename0);
        } else if (!cimg::strcasecmp(ext,"cimg") && *options) {

          // Part of a .cimg file (non-compressed).
          float
            n0 = -1, x0 = -1, y0 = -1, z0 = -1, c0 = -1,
            n1 = -1, x1 = -1, y1 = -1, z1 = -1, c1 = -1;
          if ((cimg_sscanf(options,"%f,%f%c",
                           &n0,&n1,&end)==2 ||
               cimg_sscanf(options,"%f,%f,%f,%f%c",
                           &n0,&n1,&x0,&x1,&end)==4 ||
               cimg_sscanf(options,"%f,%f,%f,%f,%f,%f%c",
                           &n0,&n1,&x0,&y0,&x1,&y1,&end)==6 ||
               cimg_sscanf(options,"%f,%f,%f,%f,%f,%f,%f,%f%c",
                           &n0,&n1,&x0,&y0,&z0,&x1,&y1,&z1,&end)==8 ||
               cimg_sscanf(options,"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f%c",
                           &n0,&n1,&x0,&y0,&z0,&c0,&x1,&y1,&z1,&c1,&end)==10) &&
              (n0==-1 || n0>=0) && (n1==-1 || n1>=0) &&
              (x0==-1 || x0>=0) && (x1==-1 || x1>=0) &&
              (y0==-1 || y0>=0) && (y1==-1 || y1>=0) &&
              (z0==-1 || z0>=0) && (z1==-1 || z1>=0) &&
              (c0==-1 || c0>=0) && (c1==-1 || c1>=0)) {
            n0 = cimg::round(n0); n1 = cimg::round(n1);
            x0 = cimg::round(x0); x1 = cimg::round(x1);
            y0 = cimg::round(y0); y1 = cimg::round(y1);
            z0 = cimg::round(z0); z1 = cimg::round(z1);
            c0 = cimg::round(c0); c1 = cimg::round(c1);
            if (c0==-1 && c1==-1) {
              if (z0==-1 && z1==-1) {
                if (y0==-1 && y1==-1) {
                  if (x0==-1 && x1==-1) {
                    print(images,0,"Input crop [%d] -> [%d] of file '%s' at position%s",
                          (int)n0,(int)n1,
                          _filename0,_gmic_selection.data());
                    input_images.load_cimg(filename,
                                           (unsigned int)n0,(unsigned int)n1,
                                           0U,0U,0U,0U,~0U,~0U,~0U,~0U);
                  } else {
                    print(images,0,"Input crop [%d](%d) -> [%d](%d) of file '%s' at position%s",
                          (int)n0,(int)x0,(int)n1,(int)x1,
                          _filename0,_gmic_selection.data());
                    input_images.load_cimg(filename,
                                           (unsigned int)n0,(unsigned int)n1,
                                           (unsigned int)x0,0U,0U,0U,
                                           (unsigned int)x1,~0U,~0U,~0U);
                  }
                } else {
                  print(images,0,"Input crop [%d](%d,%d) -> [%d](%d,%d) of file '%s' at position%s",
                        (int)n0,(int)n1,(int)x0,(int)y0,(int)x1,(int)y1,
                        _filename0,_gmic_selection.data());
                  input_images.load_cimg(filename,
                                         (unsigned int)n0,(unsigned int)n1,
                                         (unsigned int)x0,(unsigned int)y0,0U,0U,
                                         (unsigned int)x1,(unsigned int)y1,~0U,~0U);
                }
              } else {
                print(images,0,"Input crop [%d](%d,%d,%d) -> [%d](%d,%d,%d) of file '%s' "
                      "at position%s",
                      (int)n0,(int)n1,(int)x0,(int)y0,(int)z0,(int)x1,(int)y1,(int)z1,
                      _filename0,_gmic_selection.data());
                input_images.load_cimg(filename,
                                       (unsigned int)n0,(unsigned int)n1,
                                       (unsigned int)x0,(unsigned int)y0,(unsigned int)z0,0U,
                                       (unsigned int)x1,(unsigned int)y1,(unsigned int)z1,~0U);
              }
            } else {
                print(images,0,"Input crop [%d](%d,%d,%d,%d) -> [%d](%d,%d,%d,%d) of file '%s' "
                      "at position%s",
                      (int)n0,(int)n1,
                      (int)x0,(int)y0,(int)z0,(int)c0,
                      (int)x1,(int)y1,(int)z1,(int)c1,
                      _filename0,_gmic_selection.data());
                input_images.load_cimg(filename,
                                       (unsigned int)n0,(unsigned int)n1,
                                       (unsigned int)x0,(unsigned int)y0,
                                       (unsigned int)z0,(unsigned int)c0,
                                       (unsigned int)x1,(unsigned int)y1,
                                       (unsigned int)z1,(unsigned int)c1);
            }

            if (input_images) {
              input_images_names.insert(__filename0);
              if (input_images.size()>1)
                input_images_names.insert(input_images.size() - 1,__filename0.copymark());
            }
          } else
            error(images,0,0,
                  "Command '-input': .cimg file '%s', invalid file options '%s'.",
                  _filename0,options.data());

        } else if (!cimg::strcasecmp(ext,"gmz")) {
          print(images,0,"Input file '%s' at position%s",
                _filename0,
                _gmic_selection.data());
          input_images.load_cimg(filename);
          bool is_gmz = false;
          const CImg<char> back = input_images?CImg<char>(input_images.back()):CImg<char>::empty();
          if (back.width()==1 && back.depth()==1 && back.spectrum()==1 &&
              back[0]=='G' && back[1]=='M' && back[2]=='Z' && !back[3]) {
            input_images_names = back.get_split(CImg<char>::vector(0),0,false);
            if (input_images_names) {
              is_gmz = true;
              input_images_names.remove(0);
              cimglist_for(input_images_names,l)
                input_images_names[l].resize(1,input_images_names[l].height() + 1,1,1,0).
                unroll('x');
              input_images.remove();
            }
          }
          if (!is_gmz)
            error(images,0,0,"Command '-input': File '%s' is not in .gmz format (magic number not found).",
                  _filename0);
          if (input_images.size()!=input_images_names.size())
            error(images,0,0,"Command '-input': File '%s' is not in .gmz format "
                  "(numbers of images and names do not match).",
                  _filename0);

        } else if (!cimg::strcasecmp(ext,"cimg") || !cimg::strcasecmp(ext,"cimgz")) {
          print(images,0,"Input file '%s' at position%s",
                _filename0,
                _gmic_selection.data());
          input_images.load_cimg(filename);
          if (input_images) {
            input_images_names.insert(__filename0);
            if (input_images.size()>1)
              input_images_names.insert(input_images.size() - 1,__filename0.copymark());
          }

        } else if (!cimg::strcasecmp(ext,"avi") ||
                   !cimg::strcasecmp(ext,"mov") ||
                   !cimg::strcasecmp(ext,"asf") ||
                   !cimg::strcasecmp(ext,"divx") ||
                   !cimg::strcasecmp(ext,"flv") ||
                   !cimg::strcasecmp(ext,"mpg") ||
                   !cimg::strcasecmp(ext,"m1v") ||
                   !cimg::strcasecmp(ext,"m2v") ||
                   !cimg::strcasecmp(ext,"m4v") ||
                   !cimg::strcasecmp(ext,"mjp") ||
                   !cimg::strcasecmp(ext,"mp4") ||
                   !cimg::strcasecmp(ext,"mkv") ||
                   !cimg::strcasecmp(ext,"mpe") ||
                   !cimg::strcasecmp(ext,"movie") ||
                   !cimg::strcasecmp(ext,"ogm") ||
                   !cimg::strcasecmp(ext,"ogg") ||
                   !cimg::strcasecmp(ext,"qt") ||
                   !cimg::strcasecmp(ext,"rm") ||
                   !cimg::strcasecmp(ext,"vob") ||
                   !cimg::strcasecmp(ext,"wmv") ||
                   !cimg::strcasecmp(ext,"xvid") ||
                   !cimg::strcasecmp(ext,"mpeg")) {

          // Image sequence file.
          float first_frame = 0, last_frame = -1, step = 1;
          if ((cimg_sscanf(options,"%f,%f%c",&first_frame,&last_frame,&end)==2 ||
              cimg_sscanf(options,"%f,%f,%f%c",&first_frame,&last_frame,&step,&end)==3) &&
              first_frame>=0 && (last_frame>=first_frame || last_frame==-1) && step>=0) {

            // Read several frames.
            step = cimg::round(step);
            const unsigned int
              _first_frame = (unsigned int)first_frame,
              _last_frame = last_frame>=0?(unsigned int)last_frame:~0U;
            if (_last_frame!=~0U)
              print(images,0,"Input frames %u...%u with step %g of file '%s' at position%s",
                    _first_frame,_last_frame,step,
                    _filename0,
                    _gmic_selection.data());
            else
              print(images,0,"Input frames %u...(last) with step %g of file '%s' at position%s",
                    _first_frame,step,
                    _filename0,
                    _gmic_selection.data());

            input_images.load_video(filename,_first_frame,_last_frame,(unsigned int)step);
          } else if (cimg_sscanf(options,"%f%c",&first_frame,&end)==1 &&
                     first_frame>=0) {
            // Read a single frame.
            const unsigned int _first_frame = (unsigned int)first_frame;
            print(images,0,"Input frame %u of file '%s' at position%s",
                  _first_frame,_filename0,
                  _gmic_selection.data());
            input_images.load_video(filename,_first_frame,_first_frame);
          } else if (!*options) {
            // Read all frames.
            print(images,0,"Input all frames of file '%s' at position%s",
                  _filename0,
                  _gmic_selection.data());
            input_images.load_video(filename);
          } else
            error(images,0,0,
                  "Command '-input': video file '%s', invalid file options '%s'.",
                  _filename0,options.data());
          if (input_images) {
            input_images_names.insert(__filename0);
            if (input_images.size()>1)
              input_images_names.insert(input_images.size() - 1,__filename0.copymark());
          }
        } else if (!cimg::strcasecmp("raw",ext)) {

          // Raw file.
          float dx = 0, dy = 1, dz = 1, dc = 1;
          unsigned long offset = 0;
          *argx = 0;
          if (!*options ||
              cimg_sscanf(options,"%f%c",&dx,&end)==1 ||
              cimg_sscanf(options,"%f,%f%c",&dx,&dy,&end)==2 ||
              cimg_sscanf(options,"%f,%f,%f%c",&dx,&dy,&dz,&end)==3 ||
              cimg_sscanf(options,"%f,%f,%f,%f%c",&dx,&dy,&dz,&dc,&end)==4 ||
              cimg_sscanf(options,"%f,%f,%f,%f,%lu%c",&dx,&dy,&dz,&dc,&offset,&end)==5 ||
              cimg_sscanf(options,"%255[a-zA-Z]%c",argx,&end)==1 ||
              cimg_sscanf(options,"%255[a-zA-Z],%f%c",argx,&dx,&end)==2 ||
              cimg_sscanf(options,"%255[a-zA-Z],%f,%f%c",argx,&dx,&dy,&end)==3 ||
              cimg_sscanf(options,"%255[a-zA-Z],%f,%f,%f%c",argx,&dx,&dy,&dz,&end)==4 ||
              cimg_sscanf(options,"%255[a-zA-Z],%f,%f,%f,%f%c",argx,&dx,&dy,&dz,&dc,&end)==5 ||
              cimg_sscanf(options,"%255[a-zA-Z],%f,%f,%f,%f,%lu%c",argx,&dx,&dy,&dz,&dc,&offset,
                          &end)==6) {
            const char *const stype = *argx?argx:cimg::type<T>::string();
            dx = cimg::round(dx);
            dy = cimg::round(dy);
            dz = cimg::round(dz);
            dc = cimg::round(dc);
            if (dx<0 || dy<=0 || dz<=0 || dc<=0)
              error(images,0,0,
                    "Command '-input': raw file '%s', invalid specified "
		    "dimensions %gx%gx%gx%g.",
                    _filename0,dx,dy,dz,dc);

            if (offset)
              print(images,0,"Input raw file '%s' (offset: %lu) with type '%s' at position%s",
                    _filename0,offset,stype,
                    _gmic_selection.data());
            else
              print(images,0,"Input raw file '%s' with type '%s' at position%s",
                    _filename0,stype,
                    _gmic_selection.data());

#define gmic_load_raw(value_type,svalue_type) \
            if (!cimg::strcasecmp(stype,svalue_type)) \
              CImg<value_type>::get_load_raw(filename, \
                                             (unsigned int)dx,(unsigned int)dy, \
                                             (unsigned int)dz,(unsigned int)dc,false,false,offset).\
                move_to(input_images);
            gmic_load_raw(unsigned char,"uchar")
            else gmic_load_raw(unsigned char,"unsigned char")
              else gmic_load_raw(char,"char")
                else gmic_load_raw(unsigned short,"ushort")
                  else gmic_load_raw(unsigned short,"unsigned short")
                    else gmic_load_raw(short,"short")
                      else gmic_load_raw(unsigned int,"uint")
                        else gmic_load_raw(unsigned int,"unsigned int")
                          else gmic_load_raw(int,"int")
                            else gmic_load_raw(unsigned int,"ulong")
                              else gmic_load_raw(unsigned int,"unsigned long")
                                else gmic_load_raw(int,"long")
                                  else gmic_load_raw(float,"float")
                                    else gmic_load_raw(double,"double")
                                      else error(images,0,0,
                                                 "Command '-input': raw file '%s', "
                                                 "invalid specified pixel type '%s'.\n",
                                                 _filename0,stype);
            input_images_names.insert(__filename0);
          } else
            error(images,0,0,
                  "Command '-input': raw file '%s', invalid file options '%s'.",
                  _filename0,options.data());
        } else if (!cimg::strcasecmp("yuv",ext)) {

          // YUV file.
          float first_frame = 0, last_frame = 0, step = 1, dx = 0, dy = 1;
          int err = 0;
          if ((err=cimg_sscanf(options,"%f,%f,%f,%f,%f",
                               &dx,&dy,&first_frame,&last_frame,&step))>=1) {
            dx = cimg::round(dx);
            dy = cimg::round(dy);
            if (dx<=0 || dy<=0)
              error(images,0,0,
                    "Command '-input': YUV file '%s', invalid specified dimensions %gx%g.",
                    _filename0,dx,dy);
            first_frame = cimg::round(first_frame);
            if (err>3) { // Load multiple frames.
              last_frame = cimg::round(last_frame);
              step = cimg::round(step);
              print(images,0,"Input frames %g...%g with step %g of YUV file '%s' at position%s",
                    first_frame,last_frame,step,
                    _filename0,
                    _gmic_selection.data());
              input_images.load_yuv(filename,(unsigned int)dx,(unsigned int)dy,
                                    (unsigned int)first_frame,(unsigned int)last_frame,
                                    (unsigned int)step);
            } else if (err==3) { // Load a single frame.
              print(images,0,"Input frames %g of YUV file '%s' at position%s",
                    first_frame,
                    _filename0,
                    _gmic_selection.data());
              input_images.load_yuv(filename,(unsigned int)dx,(unsigned int)dy,
                                    (unsigned int)first_frame,(unsigned int)first_frame);
            } else { // Load all frames.
              print(images,0,"Input all frames of YUV file '%s' at position%s",
                    _filename0,
                    _gmic_selection.data());
              input_images.load_yuv(filename,(unsigned int)dx,(unsigned int)dy);
            }
            if (input_images) {
              input_images_names.insert(__filename0);
              if (input_images.size()>1)
                input_images_names.insert(input_images.size() - 1,__filename0.copymark());
            }
          } else
            error(images,0,0,
                  "Command '-input': YUV file '%s', invalid or missing file options '%s'.",
                  _filename0,options.data());

          } else if (!cimg::strcasecmp("tif",ext) || !cimg::strcasecmp("tiff",ext) ||
                     (file_type && !std::strcmp(file_type,"tif"))) {

          // TIFF file.
          float first_frame = 0, last_frame = 0, step = 1;
          int err = 0;
#ifdef cimg_use_tiff
          static const TIFFErrorHandler default_handler = TIFFSetWarningHandler(0);
          if (is_very_verbose) TIFFSetWarningHandler(default_handler);
          else TIFFSetWarningHandler(0);
#endif // #ifdef cimg_use_tiff
          if ((err=cimg_sscanf(options,"%f,%f,%f",&first_frame,&last_frame,&step))>0) {
            first_frame = cimg::round(first_frame);
            if (err>1) { // Load multiple frames.
              last_frame = cimg::round(last_frame);
              step = cimg::round(step);
              print(images,0,"Input frames %g...%g with step %g of TIFF file '%s' at position%s",
                    first_frame,last_frame,step,
                    _filename0,
                    _gmic_selection.data());
              input_images.load_tiff(filename,(unsigned int)first_frame,(unsigned int)last_frame,
                                     (unsigned int)step);
            } else if (err==1) { // Load a single frame.
              print(images,0,"Input frames %g of TIFF file '%s' at position%s",
                    first_frame,
                    _filename0,
                    _gmic_selection.data());
              input_images.load_tiff(filename,(unsigned int)first_frame,(unsigned int)first_frame);
            }
          } else { // Load all frames.
            if (*options) error(images,0,0,
                                "Command '-input': TIFF file '%s', "
                                "invalid file options '%s'.",
                                _filename0,options.data());
            print(images,0,"Input all frames of TIFF file '%s' at position%s",
                  _filename0,
                  _gmic_selection.data());
            input_images.load_tiff(filename);
          }
          if (input_images) {
            input_images_names.insert(__filename0);
            if (input_images.size()>1)
              input_images_names.insert(input_images.size() - 1,__filename0.copymark());
          }
        } else if (!cimg::strcasecmp("gmic",ext)) {

          // G'MIC command file.
          const bool add_debug_info = (*options!='0');
          print(images,0,"Input custom command file '%s'%s",
                _filename0,!add_debug_info?" without debug info":"");
          unsigned int siz = 0;
          for (unsigned int l = 0; l<512; ++l) siz+=commands[l].size();
          std::FILE *const file = cimg::fopen(filename,"rb");

          bool is_command_error = false;
          CImg<char> _status;
          status.move_to(_status); // Save status because 'add_commands' can change it.
          const int _verbosity = verbosity;
          const bool _is_debug = is_debug;
          verbosity = -1; is_debug = false;
          try {
            add_commands(file,add_debug_info?filename:0);
          } catch (...) {
            is_command_error = true;
          }
          verbosity = _verbosity; is_debug = _is_debug;
          _status.move_to(status);
          if (is_command_error) {
            if (is_network_file)
              error(images,0,0,
                    "Command '-input': Unable to load valid custom command file '%s' from network.",
                    _filename0);
            else
              error(images,0,0,
                    "Command '-input': File '%s' is not recognized as a custom command file.",
                    _filename0);
          }
          cimg::fclose(file);
          if (is_verbose) {
            unsigned int nb_added = 0;
            for (unsigned int l = 0; l<512; ++l) nb_added+=commands[l].size();
            nb_added-=siz;
            cimg::mutex(29);
            std::fprintf(cimg::output()," (added %u command%s, total %u).",
                         nb_added,nb_added>1?"s":"",siz + nb_added);
            std::fflush(cimg::output());
            cimg::mutex(29,0);
          }
          continue;
        } else {

          // Other file types.
          print(images,0,"Input file '%s' at position%s",
                _filename0,
                _gmic_selection.data());

          if (*options)
            error(images,0,0,
                  "Command '-input': File '%s', format does not take any input options (options '%s' specified).",
                  _filename0,options.data());

          try {

            try {
              input_images.load(filename);
            } catch (CImgIOException&) {
              if (is_network_file)
                error(images,0,0,
                      "Command '-input': Unable to load valid image file '%s' from network.",
                      _filename0);
              else throw;
            }

            // If .gmz file without extension, process images names anyway.
            bool is_gmz = false;
            const CImg<char> back = input_images?CImg<char>(input_images.back()):CImg<char>::empty();
            if (back.width()==1 && back.depth()==1 && back.spectrum()==1 &&
                back[0]=='G' && back[1]=='M' && back[2]=='Z' && !back[3]) {
              input_images_names = back.get_split(CImg<char>::vector(0),0,false);
              if (input_images_names) {
                is_gmz = true;
                input_images_names.remove(0);
                cimglist_for(input_images_names,l)
                  input_images_names[l].resize(1,input_images_names[l].height() + 1,1,1,0).
                  unroll('x');
                input_images.remove(input_images.size() - 1);
              }
            }

            if (input_images && !is_gmz) {
              input_images_names.insert(__filename0);
              if (input_images.size()>1) {
                input_images_names.insert(input_images.size() - 1,__filename0.copymark());
              }
            }

          } catch (CImgException&) {
            std::FILE *file = 0;
            if (!(file=std::fopen(filename,"r"))) {
              if (cimg::type<T>::string()==cimg::type<float>::string() || *ext || *filename!='-') {
                if (*filename=='-' && filename[1]) { // Check for command misspelling.
                  CImg<char>::string(filename).move_to(name);
                  char *const posb = std::strchr(name,'[');
                  if (posb) *posb = 0;  // Discard selection from the command name.

                  static const char *native_command_names[] = {
                    "a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s",
                    "t","u","_u","v","w","x","y","z",
                    "+","-","*","/","\\\\",">","<","%","^","=","sh","mv","rm","rv","<<",">>","==",">=",
                    "<=","//","**","!=","&","|",
                    "d3d","+3d","/3d","f3d","j3d","l3d","m3d","*3d","o3d","p3d","r3d","s3d","-3d",
                    "t3d","db3d","md3d","rv3d","sl3d","ss3d","div3d",
                    "append","autocrop","add","add3d","abs","and","atan2","acos","asin","atan",
                    "axes",
                    "blur","boxfilter","bsr","bsl","bilateral","break",
                    "check","check3d","crop","channels","columns","command","camera","cut","cos",
                    "convolve","correlate","color3d","col3d","cosh","continue","cumulate",
                    "cursor",
                    "done","do","debug","divide","distance","dilate","discard","double3d","denoise",
                    "deriche","dijkstra","displacement","display","display3d",
                    "endif","else","elif","endlocal","endl","echo","exec","error","endian","exp",
                    "eq","ellipse","equalize","erode","elevation3d","eigen","eikonal",
                    "fill","flood","files","focale3d","fft",
                    "ge","gt","gradient","graph","guided",
                    "histogram","hsi2rgb","hsl2rgb","hsv2rgb","hessian",
                    "input","if","image","index","invert","isoline3d","isosurface3d","inpaint",
                    "ifft",
                    "keep",
                    "local","le","lt","log","log2","log10","line","lab2rgb","label","light3d",
                    "move","mirror","mul","mutex","mod","max","min","mmul","mode3d","moded3d",
                    "map","median","mdiv","mse","mandelbrot","mul3d",
                    "name","normalize","neq","noarg","noise",
                    "output","onfail","object3d","or","opacity3d",
                    "parallel","pass","patchmatch","permute","progress","print","pow","point","polygon",
                    "plasma","primitives3d","plot",
                    "quiver","quit",
                    "remove","repeat","resize","reverse","return","rows","rotate",
                    "round","rand","rotate3d","rgb2hsi","rgb2hsl","rgb2hsv","rgb2lab",
                    "rgb2srgb","rol","ror","reverse3d",
                    "status","_status","skip","set","split","shared","shift","slices","srand","sub","sqrt",
                    "sqr","sign","sin","sort","solve","sub3d","sharpen","smooth","split3d",
                    "svd","sphere3d","specl3d","specs3d","sinc","sinh","srgb2rgb","streamline3d",
                    "structuretensors","select","serialize",
                    "threshold","tan","text","texturize3d","trisolve","tanh",
                    "unroll","uncommand","unserialize",
                    "vanvliet","verbose",
                    "while","warn","window","warp","watershed","wait",
                    "xor",0
                  };

                  const char *misspelled = 0;
                  const unsigned int foff = name[1]=='-'?2U:1U;
                  int dmin = 4;
                  for (unsigned int l = 0; native_command_names[l]; ++l) {
                    // Look in native commands.
                    const char *const c = native_command_names[l];
                    const int d = levenshtein(c,name.data() + foff);
                    if (d<dmin) { dmin = d; misspelled = native_command_names[l]; }
                  }
                  for (unsigned int i = 0; i<512; ++i)
                    // Look in custom commands.
                    cimglist_for(commands_names[i],l) {
                      const char *const c = commands_names[i][l].data();
                      const int d = levenshtein(c,name.data() + foff);
                      if (d<dmin) { dmin = d; misspelled = commands_names[i][l].data(); }
                    }
                  if (misspelled)
                    error(images,0,0,
                          "Unknown command or filename '%s' (did you mean '-%s' ?).",
                          gmic_argument_text(),misspelled);
                  else error(images,0,0,
                             "Unknown command or filename '%s'.",
                             gmic_argument_text());
                } else error(images,0,0,
                             "Unknown %s '%s'.",
                             *filename=='-'?"command or filename":"filename",
                             gmic_argument_text());
              } else
                error(images,0,0,
                      "Unknown command '%s' in '%s' type mode "
                      "(command defined only in 'float' type mode ?).",
                      gmic_argument_text(),cimg::type<T>::string());
            } else throw;
          }
        }

        if (*filename_tmp) std::remove(filename_tmp); // Clean temporary file if used.
        if (is_network_file) std::remove(_filename);  // Clean temporary file if network input.
      }

      if (is_verbose) {
        cimg::mutex(29);
        if (input_images) {
          const unsigned int last = input_images.size() - 1;
          if (input_images.size()==1) {
            if (input_images[0].is_CImg3d(false))
              std::fprintf(cimg::output()," (%u vertices, %u primitives).",
                           cimg::float2uint((float)input_images(0,6)),
                           cimg::float2uint((float)input_images(0,7)));
            else
              std::fprintf(cimg::output()," (1 image %dx%dx%dx%d).",
                           input_images[0].width(),input_images[0].height(),
                           input_images[0].depth(),input_images[0].spectrum());
          } else
            std::fprintf(cimg::output()," (%u images [0] = %dx%dx%dx%d, %s[%u] = %dx%dx%dx%d).",
                         input_images.size(),
                         input_images[0].width(),input_images[0].height(),
                         input_images[0].depth(),input_images[0].spectrum(),
                         last==1?"":"(...),",last,
                         input_images[last].width(),input_images[last].height(),
                         input_images[last].depth(),input_images[last].spectrum());
        } else {
          std::fprintf(cimg::output()," (no available data).");
          input_images_names.assign();
        }
        std::fflush(cimg::output());
        cimg::mutex(29,0);
      }

      for (unsigned int l = 0, siz = selection.height() - 1U, off = 0; l<=siz; ++l) {
        const unsigned int uind = selection[l] + off;
        off+=input_images.size();
        if (l!=siz) {
          images.insert(input_images,uind);
          images_names.insert(input_images_names,uind);
        } else {
          input_images.move_to(images,uind);
          input_images_names.move_to(images_names,uind);
        }
      }

      if (new_name) new_name.move_to(images_names[selection[0]]);
      is_released = false;
    } // End main parsing loop of _run().

    // Wait for remaining threads to finish.
#ifdef gmic_is_parallel
    cimglist_for(threads_data,i) cimg_forY(threads_data[i],l) {
      if (!threads_data(i,l).is_thread_running) threads_data(i,l).gmic_instance.is_abort_thread = true;
#if cimg_OS!=2
      pthread_join(threads_data(i,l).thread_id,0);
#else // #if cimg_OS!=2
      WaitForSingleObject(threads_data(i,l).thread_id,INFINITE);
      CloseHandle(threads_data(i,l).thread_id);
#endif // #if cimg_OS!=2
      is_released&=threads_data(i,l).gmic_instance.is_released;
      threads_data(i,l).is_thread_running = false;
    }
    // Check for possible exceptions thrown by threads.
    cimglist_for(threads_data,i) cimg_forY(threads_data[i],l)
      if (threads_data(i,l).exception._message)
        throw threads_data(i,l).exception;
#endif // #ifdef gmic_is_parallel

    // Post-check global environment consistency.
    if (images_names.size()!=images.size())
      error(images,0,0,
            "Internal error: Images (%u) and images names (%u) have different size, "
	    "at return point.",
            images_names.size(),images.size());
    if (!callstack)
      error(images,0,0,
            "Internal error: Empty call stack at return point.");

    // Post-check local environment consistency.
    if (!is_quit && !is_return) {
      const CImg<char>& s = callstack.back();
      if (s[0]=='*' && (s[1]=='d' || s[1]=='i' || s[1]=='r' || (s[1]=='l' && !is_endlocal))) {
        unsigned int reference_line = ~0U;
        if (cimg_sscanf(s,"*%*[a-z]#%u",&reference_line)==1)
          error(images,0,0,
                "A '-%s' command is missing (for '-%s', line #%u), before return point.",
                s[1]=='d'?"while":s[1]=='i'?"endif":s[1]=='r'?"done":"endlocal",
                s[1]=='d'?"do":s[1]=='i'?"if":s[1]=='r'?"repeat":"local",
                reference_line);
        else error(images,0,0,
              "A '-%s' command is missing, before return point.",
              s[1]=='d'?"while":s[1]=='i'?"endif":s[1]=='r'?"done":"endlocal");
      }
    } else if (initial_callstack_size<callstack.size()) callstack.remove(initial_callstack_size,callstack.size() - 1);

    // Post-check validity of shared images.
    cimglist_for(images,l) gmic_check(images[l]);

    // Display or print result, if not 'released' before.
    if (!is_released && callstack.size()==1 && images) {
#ifdef gmic_main
#if cimg_display!=0
      CImgList<unsigned int> lselection, lselection3d;
      bool is_first3d = false;
      _display_window[0].assign();
      cimglist_for(images,l) {
        const bool is_3d = images[l].is_CImg3d(false);
        if (!l) is_first3d = is_3d;
        CImg<unsigned int>::vector(l).move_to(is_3d?lselection3d:lselection);
      }
      if (is_first3d) {
        display_objects3d(images,images_names,lselection3d>'y',CImg<unsigned char>::empty(),false);
        if (lselection) display_images(images,images_names,lselection>'y',0,false);
      } else {
        if (lselection) display_images(images,images_names,lselection>'y',0,false);
        if (lselection3d) display_objects3d(images,images_names,lselection3d>'y',CImg<unsigned char>::empty(),false);
      }
#else
      CImg<unsigned int> seq(1,images.width());
      cimg_forY(seq,y) seq[y] = y;
      print_images(images,images_names,seq,true);
#endif // #if cimg_display!=0
#endif // #ifdef gmic_main
      is_released = true;
    }

    if (is_debug) debug(images,"%sExit scope '%s/'.%s\n",
                        cimg::t_bold,callstack.back().data(),cimg::t_normal);

    if (callstack.size()==1) {
      if (is_quit) {
        if (verbosity>=0 || is_debug) {
          std::fputc('\n',cimg::output());
          std::fflush(cimg::output());
        }
      } else {
        print(images,0,"End G'MIC interpreter.\n");
        is_quit = true;
      }
    }
  } catch (CImgAbortException &) { // Special case of abort (abort from a CImg method).
    // Do the same as for a cancellation point.
    const bool is_very_verbose = verbosity>0 || is_debug;
    if (is_very_verbose) print(images,0,"Abort G'MIC interpreter.");
    dowhiles.assign();
    repeatdones.assign();
    position = commands_line.size();
    is_released = is_quit = true;
  } catch (CImgException &e) {
    CImg<char> error_message(e.what(),(unsigned int)std::strlen(e.what()) + 1);
    for (char *str = std::strstr(error_message,"CImg<"); str; str = std::strstr(str,"CImg<")) {
      str[0] = 'g'; str[1] = 'm'; str[2] = 'i'; str[3] = 'c';
    }
    for (char *str = std::strstr(error_message,"cimg:"); str; str = std::strstr(str,"cimg:")) {
      str[0] = 'g'; str[1] = 'm'; str[2] = 'i'; str[3] = 'c';
    }
    if (*current_command && current_command[1])
      error(images,0,current_command + 1,"Command '%s': %s",current_command,error_message.data());
    else
      error(images,0,0,"%s",error_message.data());
  }

  if (!is_endlocal) debug_line = initial_debug_line;
  else {
    if (next_debug_line!=~0U) { debug_line = next_debug_line; next_debug_line = ~0U; }
    if (next_debug_filename!=~0U) { debug_filename = next_debug_filename; next_debug_filename = ~0U; }
  }

  return *this;
}

// Fallback function for segfault signals.
#if cimg_OS==1
void gmic_segfault_sigaction(int signal, siginfo_t *si, void *arg) {
  cimg::unused(signal,si,arg);
  cimg::mutex(29);
  std::fprintf(cimg::output(),
               "\n\n%s[gmic] G'MIC encountered a %sfatal error%s%s (Segmentation fault). "
               "Please submit a bug report, at: %shttps://github.com/dtschump/gmic/issues%s\n\n",
               cimg::t_red,cimg::t_bold,cimg::t_normal,cimg::t_red,
               cimg::t_bold,cimg::t_normal);
  std::fflush(cimg::output());
  cimg::mutex(29,0);
  std::exit(EXIT_FAILURE);
}
#endif

//----------------------
// Start CLI interface.
//----------------------
#ifdef gmic_main

#if cimg_OS==2
int _CRT_glob = 0; // Disable globbing for msys.
#endif

int main(int argc, char **argv) {

  // Set default output messages stream.
  const char *const is_debug = cimg_option("-debug",(char*)0,0);
  cimg::output(is_debug?stdout:stderr);

  // Set fallback for segfault signals.
#if cimg_OS==1
  struct sigaction sa;
  std::memset(&sa,0,sizeof(sa));
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = gmic_segfault_sigaction;
  sa.sa_flags = SA_SIGINFO;
  sigaction(SIGSEGV,&sa,0);
#endif

  // Create resources directory.
  if (!gmic::init_rc()) {
    std::fprintf(cimg::output(),
                 "\n[gmic] Unable to create resources directory 'gmicrc'.\n");
    std::fflush(cimg::output());
  }

  // Set special path for curl on Windows
  // (in case the use of libcurl is not enabled).
#if cimg_OS==2
  cimg::curl_path("_gmic\\curl",true);
#endif

  // Declare main G'MIC instance.
  gmic gmic_instance;
  gmic_instance.add_commands("cli_start : ");

  // Load startup command files.
  CImg<char> commands_user, commands_update, filename_update;
  bool is_invalid_user = false, is_invalid_update = false;
  char sep = 0;
  cimg::exception_mode(0);
  gmic_instance.verbosity = -1;

  // Update file (in resources directory).
  filename_update.assign(1024);
  cimg_snprintf(filename_update,filename_update.width(),"%scli_update%u.gmic",
                gmic::path_rc(),gmic_version);
  try {
    commands_update.load_raw(filename_update).append(CImg<char>::vector(0),'y');
    try { gmic_instance.add_commands(commands_update);
    } catch (...) { is_invalid_update = true; throw; }
  } catch (...) { commands_update.assign(); }
  if (commands_update && (cimg_sscanf(commands_update," #@gmi%c",&sep)!=1 || sep!='c'))
    commands_update.assign(); // Discard invalid update file.

  // User file (in parent of resources directory).
  const char *const filename_user = gmic::path_user();
  try {
    commands_user.load_raw(filename_user).append(CImg<char>::vector(0),'y');
    try { gmic_instance.add_commands(commands_user,filename_user);
    } catch (...) { is_invalid_user = true; throw; }
  } catch (...) { commands_user.assign(); }

  // When help has been requested.
  const char
    *const is_help1 = cimg_option("-h",(char*)0,0),
    *const is_help2 = cimg_option("--h",(char*)0,0),
    *const is_help3 = cimg_option("-help",(char*)0,0),
    *const is_help4 = cimg_option("--help",(char*)0,0),
    *const help_command = is_help1?"-h":is_help2?"--h":is_help3?"-help":"--help",
    *const help_argument = is_help1?is_help1:is_help2?is_help2:is_help3?is_help3:is_help4;
  const bool
    is_help = is_help1 || is_help2 || is_help3 || is_help4,
    is_global_help = is_help && !std::strcmp(help_command,help_argument);

  if (is_help) {

    // Load specified commands definitions data (eventually).
    CImgList<gmic_pixel_type> images;
    CImgList<char> images_names;
    if (!is_global_help && commands_user) commands_user.move_to(images);
    if (commands_update) images.insert(commands_update);
    if (!is_global_help || !commands_update) images.insert(gmic::stdlib);
    commands_update.assign();

    for (int i = 1; i<argc; ++i) {
      std::FILE *file = 0;
      CImg<char> filename_tmp(256); *filename_tmp = 0;
      if ((!std::strcmp("-m",argv[i]) || !std::strcmp("-command",argv[i])) && i<argc - 1) {
        const char *const filename = argv[++i];
        if (!cimg::strncasecmp(filename,"http://",7) || !cimg::strncasecmp(filename,"https://",8))
          try {
            file = std::fopen(cimg::load_network(filename,filename_tmp),"r");
          } catch (CImgException&) { file = 0; }
        else file = std::fopen(filename,"r");
      } else if (!cimg::strcasecmp("gmic",cimg::split_filename(argv[i]))) {
        const char *const filename = argv[i];
        if (!cimg::strncasecmp(filename,"http://",7) || !cimg::strncasecmp(filename,"https://",8))
          try {
            file = std::fopen(cimg::load_network(filename,filename_tmp),"r");
          } catch (CImgException&) { file = 0; }
        else file = std::fopen(filename,"r");
      }
      if (file) {
        const unsigned int n = images.size();
        try {
          CImg<unsigned char>::get_load_cimg(file).move_to(images,0);
        } catch (CImgIOException&) {
          CImg<unsigned char>::get_load_raw(file).move_to(images,0);
        }
        if (images.size()!=n) CImg<unsigned char>::vector('\n').move_to(images,1);
        cimg::fclose(file);
        if (*filename_tmp) std::remove(filename_tmp);
      }
    }

    cimg::output(stdout);
    if (is_global_help) { // Global help.
      try {
        gmic_instance.verbosity = -1;
        gmic_instance.run("-v - -l -help \"\" -onfail -endl -q",images,images_names);
      } catch (...) { // Fallback in case default version of '-help' has been overloaded.
        images.assign();
        images_names.assign();
        images.insert(gmic::stdlib);
        gmic("-v - -l -help \"\" -onfail -endl -q",images,images_names);
      }
    } else { // Help for a specified command.
      CImg<char> tmp_line(1024);
      try {
        cimg_snprintf(tmp_line,tmp_line.width(),"-v - -l -help \"%s\",1 -onfail -endl -q",help_argument);
        gmic_instance.verbosity = -1;
        gmic_instance.run(tmp_line,images,images_names);
      } catch (...) { // Fallback in case default version of '-help' has been overloaded.
        cimg_snprintf(tmp_line,tmp_line.width(),"-v - -l -help \"%s\",1 -onfail -endl -q",help_argument);
        images.assign();
        images_names.assign();
        images.insert(gmic::stdlib);
        gmic(tmp_line,images,images_names);
      }
    }

    std::exit(0);
  }

  // Convert 'argv' into G'MIC command line.
  commands_user.assign(); commands_update.assign();

  CImgList<char> items;
  if (argc==1) { // When no args have been specified.
    gmic_instance.verbosity = -1;
    CImg<char>::string("-l[] -cli_noarg -onfail -endl").move_to(items);
  } else {
    gmic_instance.verbosity = 0;
    for (int l = 1; l<argc; ++l) { // Split argv as items.
      if (std::strchr(argv[l],' ')) {
        CImg<char>::vector('\"').move_to(items);
        CImg<char>(argv[l],(unsigned int)std::strlen(argv[l])).move_to(items);
        CImg<char>::string("\"").move_to(items);
      } else CImg<char>::string(argv[l]).move_to(items);
      if (l<argc - 1) items.back().back()=' ';
    }
  }

  // Insert startup command.
  const bool is_first_item_verbose = items.width()>1 &&
    (!std::strncmp("-v ",items[0],3) || !std::strncmp("-verbose ",items[0],9));
  items.insert(CImg<char>::string("-cli_start ",false),is_first_item_verbose?2:0);

  if (is_invalid_user) { // Display warning message in case of invalid user command file.
    CImg<char> tmpstr(1024);
    cimg_snprintf(tmpstr,tmpstr.width(),"-warn \"File '%s' is not a valid G'MIC command file.\" ",
                  filename_user);
    items.insert(CImg<char>::string(tmpstr.data(),false),is_first_item_verbose?2:0);
  }
  if (is_invalid_update) { // Display warning message in case of invalid user command file.
    CImg<char> tmpstr(1024);
    cimg_snprintf(tmpstr,tmpstr.width(),"-warn \"File '%s' is not a valid G'MIC command file.\" ",
                  filename_update.data());
    items.insert(CImg<char>::string(tmpstr.data(),false),is_first_item_verbose?2:0);
  }

  const CImg<char> commands_line(items>'x');
  items.assign();

  // Launch G'MIC interpreter.
  try {
    CImgList<gmic_pixel_type> images;
    CImgList<char> images_names;
    gmic_instance.run(commands_line.data(),images,images_names);
  } catch (gmic_exception &e) {

    // Something went wrong during the pipeline execution.
    if (gmic_instance.verbosity<0) {
      std::fprintf(cimg::output(),"\n[gmic] %s%s%s%s",
                   cimg::t_red,cimg::t_bold,
                   e.what(),cimg::t_normal);
      std::fflush(cimg::output());
    }
    if (*e.command_help()) {
      std::fprintf(cimg::output(),"\n[gmic] Command '-%s' has the following description: \n",
		   e.command_help());
      std::fflush(cimg::output());
      CImgList<gmic_pixel_type> images;
      CImgList<char> images_names;
      images.insert(gmic::stdlib);
      CImg<char> tmp_line(1024);
      cimg_snprintf(tmp_line,tmp_line.width(),
                    "-v - "
                    "-l[] -i raw:\"%s\",char -m \"%s\" -onfail -rm -endl "
                    "-l[] -i raw:\"%s\",char -m \"%s\" -onfail -rm -endl "
                    "-rv -help \"%s\",0 -q",
                    filename_update.data(),filename_update.data(),
                    filename_user,filename_user,
                    e.command_help());
      try {
        gmic(tmp_line,images,images_names);
      } catch (...) {
        cimg_snprintf(tmp_line,tmp_line.width(),"-v - -help \"%s\",1 -q",e.command_help());
        images.assign();
        images_names.assign();
        images.insert(gmic::stdlib);
        gmic(tmp_line,images,images_names);
      }
    } else { std::fprintf(cimg::output(),"\n\n"); std::fflush(cimg::output()); }
    return -1;
  }
  return 0;
}

#else

// Explicitely instanciate constructors and destructor when building the library.
template gmic::gmic(const char *const commands_line,
                    gmic_list<gmic_pixel_type>& images, gmic_list<char>& images_names,
                    const char *const custom_commands, const bool include_stdlib,
                    float *const p_progress, bool *const p_is_abort);

template gmic& gmic::run(const char *const commands_line,
                         gmic_list<gmic_pixel_type> &images, gmic_list<char> &images_names,
                         float *const p_progress=0, bool *const p_is_abort=0);

template CImgList<float>::~CImgList();
template CImgList<char>::~CImgList();

#endif // #ifdef gmic_main
#endif // #ifdef cimg_plugin
