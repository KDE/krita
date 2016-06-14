/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QOPENGL_EXTENSIONS_P_H
#define QOPENGL_EXTENSIONS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the Qt OpenGL classes.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "qopenglextrafunctions.h"
#include <QtCore/qlibrary.h>

QT_BEGIN_NAMESPACE

class QOpenGLExtensionsPrivate;

class QOpenGLES3Helper
{
public:
    QOpenGLES3Helper();

    // GLES3
    void (QOPENGLF_APIENTRYP ReadBuffer)(GLenum mode);
    void (QOPENGLF_APIENTRYP DrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
    void (QOPENGLF_APIENTRYP TexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
    void (QOPENGLF_APIENTRYP TexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
    void (QOPENGLF_APIENTRYP CopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP CompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
    void (QOPENGLF_APIENTRYP CompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
    void (QOPENGLF_APIENTRYP GenQueries)(GLsizei n, GLuint *ids);
    void (QOPENGLF_APIENTRYP DeleteQueries)(GLsizei n, const GLuint *ids);
    GLboolean (QOPENGLF_APIENTRYP IsQuery)(GLuint id);
    void (QOPENGLF_APIENTRYP BeginQuery)(GLenum target, GLuint id);
    void (QOPENGLF_APIENTRYP EndQuery)(GLenum target);
    void (QOPENGLF_APIENTRYP GetQueryiv)(GLenum target, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetQueryObjectuiv)(GLuint id, GLenum pname, GLuint *params);
    GLboolean (QOPENGLF_APIENTRYP UnmapBuffer)(GLenum target);
    void (QOPENGLF_APIENTRYP GetBufferPointerv)(GLenum target, GLenum pname, void **params);
    void (QOPENGLF_APIENTRYP DrawBuffers)(GLsizei n, const GLenum *bufs);
    void (QOPENGLF_APIENTRYP UniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP UniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP UniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP UniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP UniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP UniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP BlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
    void (QOPENGLF_APIENTRYP RenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP FramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
    void *(QOPENGLF_APIENTRYP MapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
    void (QOPENGLF_APIENTRYP FlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length);
    void (QOPENGLF_APIENTRYP BindVertexArray)(GLuint array);
    void (QOPENGLF_APIENTRYP DeleteVertexArrays)(GLsizei n, const GLuint *arrays);
    void (QOPENGLF_APIENTRYP GenVertexArrays)(GLsizei n, GLuint *arrays);
    GLboolean (QOPENGLF_APIENTRYP IsVertexArray)(GLuint array);
    void (QOPENGLF_APIENTRYP GetIntegeri_v)(GLenum target, GLuint index, GLint *data);
    void (QOPENGLF_APIENTRYP BeginTransformFeedback)(GLenum primitiveMode);
    void (QOPENGLF_APIENTRYP EndTransformFeedback)(void);
    void (QOPENGLF_APIENTRYP BindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
    void (QOPENGLF_APIENTRYP BindBufferBase)(GLenum target, GLuint index, GLuint buffer);
    void (QOPENGLF_APIENTRYP TransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
    void (QOPENGLF_APIENTRYP GetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
    void (QOPENGLF_APIENTRYP VertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
    void (QOPENGLF_APIENTRYP GetVertexAttribIiv)(GLuint index, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint *params);
    void (QOPENGLF_APIENTRYP VertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w);
    void (QOPENGLF_APIENTRYP VertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
    void (QOPENGLF_APIENTRYP VertexAttribI4iv)(GLuint index, const GLint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI4uiv)(GLuint index, const GLuint *v);
    void (QOPENGLF_APIENTRYP GetUniformuiv)(GLuint program, GLint location, GLuint *params);
    GLint (QOPENGLF_APIENTRYP GetFragDataLocation)(GLuint program, const GLchar *name);
    void (QOPENGLF_APIENTRYP Uniform1ui)(GLint location, GLuint v0);
    void (QOPENGLF_APIENTRYP Uniform2ui)(GLint location, GLuint v0, GLuint v1);
    void (QOPENGLF_APIENTRYP Uniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2);
    void (QOPENGLF_APIENTRYP Uniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
    void (QOPENGLF_APIENTRYP Uniform1uiv)(GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP Uniform2uiv)(GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP Uniform3uiv)(GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP Uniform4uiv)(GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP ClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint *value);
    void (QOPENGLF_APIENTRYP ClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint *value);
    void (QOPENGLF_APIENTRYP ClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
    const GLubyte *(QOPENGLF_APIENTRYP GetStringi)(GLenum name, GLuint index);
    void (QOPENGLF_APIENTRYP CopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    void (QOPENGLF_APIENTRYP GetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
    void (QOPENGLF_APIENTRYP GetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
    GLuint (QOPENGLF_APIENTRYP GetUniformBlockIndex)(GLuint program, const GLchar *uniformBlockName);
    void (QOPENGLF_APIENTRYP GetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
    void (QOPENGLF_APIENTRYP UniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
    void (QOPENGLF_APIENTRYP DrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
    void (QOPENGLF_APIENTRYP DrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
    GLsync (QOPENGLF_APIENTRYP FenceSync)(GLenum condition, GLbitfield flags);
    GLboolean (QOPENGLF_APIENTRYP IsSync)(GLsync sync);
    void (QOPENGLF_APIENTRYP DeleteSync)(GLsync sync);
    GLenum (QOPENGLF_APIENTRYP ClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
    void (QOPENGLF_APIENTRYP WaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
    void (QOPENGLF_APIENTRYP GetInteger64v)(GLenum pname, GLint64 *data);
    void (QOPENGLF_APIENTRYP GetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
    void (QOPENGLF_APIENTRYP GetInteger64i_v)(GLenum target, GLuint index, GLint64 *data);
    void (QOPENGLF_APIENTRYP GetBufferParameteri64v)(GLenum target, GLenum pname, GLint64 *params);
    void (QOPENGLF_APIENTRYP GenSamplers)(GLsizei count, GLuint *samplers);
    void (QOPENGLF_APIENTRYP DeleteSamplers)(GLsizei count, const GLuint *samplers);
    GLboolean (QOPENGLF_APIENTRYP IsSampler)(GLuint sampler);
    void (QOPENGLF_APIENTRYP BindSampler)(GLuint unit, GLuint sampler);
    void (QOPENGLF_APIENTRYP SamplerParameteri)(GLuint sampler, GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP SamplerParameteriv)(GLuint sampler, GLenum pname, const GLint *param);
    void (QOPENGLF_APIENTRYP SamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param);
    void (QOPENGLF_APIENTRYP SamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat *param);
    void (QOPENGLF_APIENTRYP GetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP VertexAttribDivisor)(GLuint index, GLuint divisor);
    void (QOPENGLF_APIENTRYP BindTransformFeedback)(GLenum target, GLuint id);
    void (QOPENGLF_APIENTRYP DeleteTransformFeedbacks)(GLsizei n, const GLuint *ids);
    void (QOPENGLF_APIENTRYP GenTransformFeedbacks)(GLsizei n, GLuint *ids);
    GLboolean (QOPENGLF_APIENTRYP IsTransformFeedback)(GLuint id);
    void (QOPENGLF_APIENTRYP PauseTransformFeedback)(void);
    void (QOPENGLF_APIENTRYP ResumeTransformFeedback)(void);
    void (QOPENGLF_APIENTRYP GetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
    void (QOPENGLF_APIENTRYP ProgramBinary)(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
    void (QOPENGLF_APIENTRYP ProgramParameteri)(GLuint program, GLenum pname, GLint value);
    void (QOPENGLF_APIENTRYP InvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum *attachments);
    void (QOPENGLF_APIENTRYP InvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP TexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP TexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
    void (QOPENGLF_APIENTRYP GetInternalformativ)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);

    // GLES 3.1
    void (QOPENGLF_APIENTRYP DispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
    void (QOPENGLF_APIENTRYP DispatchComputeIndirect)(GLintptr indirect);
    void (QOPENGLF_APIENTRYP DrawArraysIndirect)(GLenum mode, const void *indirect);
    void (QOPENGLF_APIENTRYP DrawElementsIndirect)(GLenum mode, GLenum type, const void *indirect);
    void (QOPENGLF_APIENTRYP FramebufferParameteri)(GLenum target, GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP GetFramebufferParameteriv)(GLenum target, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetProgramInterfaceiv)(GLuint program, GLenum programInterface, GLenum pname, GLint *params);
    GLuint (QOPENGLF_APIENTRYP GetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar *name);
    void (QOPENGLF_APIENTRYP GetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
    void (QOPENGLF_APIENTRYP GetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
    GLint (QOPENGLF_APIENTRYP GetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar *name);
    void (QOPENGLF_APIENTRYP UseProgramStages)(GLuint pipeline, GLbitfield stages, GLuint program);
    void (QOPENGLF_APIENTRYP ActiveShaderProgram)(GLuint pipeline, GLuint program);
    GLuint (QOPENGLF_APIENTRYP CreateShaderProgramv)(GLenum type, GLsizei count, const GLchar *const*strings);
    void (QOPENGLF_APIENTRYP BindProgramPipeline)(GLuint pipeline);
    void (QOPENGLF_APIENTRYP DeleteProgramPipelines)(GLsizei n, const GLuint *pipelines);
    void (QOPENGLF_APIENTRYP GenProgramPipelines)(GLsizei n, GLuint *pipelines);
    GLboolean (QOPENGLF_APIENTRYP IsProgramPipeline)(GLuint pipeline);
    void (QOPENGLF_APIENTRYP GetProgramPipelineiv)(GLuint pipeline, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP ProgramUniform1i)(GLuint program, GLint location, GLint v0);
    void (QOPENGLF_APIENTRYP ProgramUniform2i)(GLuint program, GLint location, GLint v0, GLint v1);
    void (QOPENGLF_APIENTRYP ProgramUniform3i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
    void (QOPENGLF_APIENTRYP ProgramUniform4i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    void (QOPENGLF_APIENTRYP ProgramUniform1ui)(GLuint program, GLint location, GLuint v0);
    void (QOPENGLF_APIENTRYP ProgramUniform2ui)(GLuint program, GLint location, GLuint v0, GLuint v1);
    void (QOPENGLF_APIENTRYP ProgramUniform3ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
    void (QOPENGLF_APIENTRYP ProgramUniform4ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
    void (QOPENGLF_APIENTRYP ProgramUniform1f)(GLuint program, GLint location, GLfloat v0);
    void (QOPENGLF_APIENTRYP ProgramUniform2f)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
    void (QOPENGLF_APIENTRYP ProgramUniform3f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    void (QOPENGLF_APIENTRYP ProgramUniform4f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void (QOPENGLF_APIENTRYP ProgramUniform1iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform2iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform3iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform4iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform1uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform2uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform3uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform4uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform1fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniform2fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniform3fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniform4fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix2x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix3x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix2x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix4x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix3x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix4x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ValidateProgramPipeline)(GLuint pipeline);
    void (QOPENGLF_APIENTRYP GetProgramPipelineInfoLog)(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    void (QOPENGLF_APIENTRYP BindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
    void (QOPENGLF_APIENTRYP GetBooleani_v)(GLenum target, GLuint index, GLboolean *data);
    void (QOPENGLF_APIENTRYP MemoryBarrierFunc)(GLbitfield barriers);
    void (QOPENGLF_APIENTRYP MemoryBarrierByRegion)(GLbitfield barriers);
    void (QOPENGLF_APIENTRYP TexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    void (QOPENGLF_APIENTRYP GetMultisamplefv)(GLenum pname, GLuint index, GLfloat *val);
    void (QOPENGLF_APIENTRYP SampleMaski)(GLuint maskNumber, GLbitfield mask);
    void (QOPENGLF_APIENTRYP GetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP BindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
    void (QOPENGLF_APIENTRYP VertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
    void (QOPENGLF_APIENTRYP VertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    void (QOPENGLF_APIENTRYP VertexAttribBinding)(GLuint attribindex, GLuint bindingindex);
    void (QOPENGLF_APIENTRYP VertexBindingDivisor)(GLuint bindingindex, GLuint divisor);

    QPair<int, int> supportedVersion() const { return m_supportedVersion; }

private:
    bool init();
    QFunctionPointer resolve(const char *name);
#ifndef QT_NO_LIBRARY
    QLibrary m_gl;
#endif
    QPair<int, int> m_supportedVersion;
};

class Q_GUI_EXPORT QOpenGLExtensions : public QOpenGLExtraFunctions
{
    Q_DECLARE_PRIVATE(QOpenGLExtensions)
public:
    QOpenGLExtensions();
    QOpenGLExtensions(QOpenGLContext *context);
    ~QOpenGLExtensions() {}

    enum OpenGLExtension {
        TextureRectangle        = 0x00000001,
        GenerateMipmap          = 0x00000002,
        TextureCompression      = 0x00000004,
        MirroredRepeat          = 0x00000008,
        FramebufferMultisample  = 0x00000010,
        StencilTwoSide          = 0x00000020,
        StencilWrap             = 0x00000040,
        PackedDepthStencil      = 0x00000080,
        NVFloatBuffer           = 0x00000100,
        PixelBufferObject       = 0x00000200,
        FramebufferBlit         = 0x00000400,
        BGRATextureFormat       = 0x00000800,
        DDSTextureCompression   = 0x00001000,
        ETC1TextureCompression  = 0x00002000,
        PVRTCTextureCompression = 0x00004000,
        ElementIndexUint        = 0x00008000,
        Depth24                 = 0x00010000,
        SRGBFrameBuffer         = 0x00020000,
        MapBuffer               = 0x00040000,
        GeometryShaders         = 0x00080000,
        MapBufferRange          = 0x00100000,
        Sized8Formats           = 0x00200000,
        DiscardFramebuffer      = 0x00400000,
        Sized16Formats          = 0x00800000
    };
    Q_DECLARE_FLAGS(OpenGLExtensions, OpenGLExtension)

    OpenGLExtensions openGLExtensions();
    bool hasOpenGLExtension(QOpenGLExtensions::OpenGLExtension extension) const;

    GLvoid *glMapBuffer(GLenum target, GLenum access);
    void glGetBufferSubData(GLenum target, qopengl_GLintptr offset, qopengl_GLsizeiptr size, GLvoid *data);
    void glDiscardFramebufferEXT (GLenum target, GLsizei numAttachments, const GLenum *attachments);

    QOpenGLES3Helper *gles3Helper();

    void flushShared();

private:
    static bool isInitialized(const QOpenGLFunctionsPrivate *d) { return d != 0; }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QOpenGLExtensions::OpenGLExtensions)

class QOpenGLExtensionsPrivate : public QOpenGLExtraFunctionsPrivate
{
public:
    explicit QOpenGLExtensionsPrivate(QOpenGLContext *ctx);

    GLvoid* (QOPENGLF_APIENTRYP MapBuffer)(GLenum target, GLenum access);
    void (QOPENGLF_APIENTRYP GetBufferSubData)(GLenum target, qopengl_GLintptr offset, qopengl_GLsizeiptr size, GLvoid *data);
    void (QOPENGLF_APIENTRYP DiscardFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum *attachments);

    bool flushVendorChecked;
    bool flushIsSufficientToSyncContexts;
};

inline GLvoid *QOpenGLExtensions::glMapBuffer(GLenum target, GLenum access)
{
    Q_D(QOpenGLExtensions);
    Q_ASSERT(QOpenGLExtensions::isInitialized(d));
    GLvoid *result = d->MapBuffer(target, access);
    Q_OPENGL_FUNCTIONS_DEBUG
    return result;
}

inline void QOpenGLExtensions::glGetBufferSubData(GLenum target, qopengl_GLintptr offset, qopengl_GLsizeiptr size, GLvoid *data)
{
    Q_D(QOpenGLExtensions);
    Q_ASSERT(QOpenGLExtensions::isInitialized(d));
    d->GetBufferSubData(target, offset, size, data);
    Q_OPENGL_FUNCTIONS_DEBUG
}


inline void QOpenGLExtensions::glDiscardFramebufferEXT (GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    Q_D(QOpenGLExtensions);
    Q_ASSERT(QOpenGLExtensions::isInitialized(d));
    d->DiscardFramebuffer(target,numAttachments, attachments);
    Q_OPENGL_FUNCTIONS_DEBUG
}
QT_END_NAMESPACE

#endif // QOPENGL_EXTENSIONS_P_H
