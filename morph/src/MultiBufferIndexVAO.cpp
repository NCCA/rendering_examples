#include "MultiBufferIndexVAO.h"
#include <iostream>

void MultiBufferIndexVAO::draw() const
{
  if(m_allocated == false)
  {
    std::cerr<<"Warning trying to draw an unallocated VOA\n";
  }
  if(m_bound == false)
  {
    std::cerr<<"Warning trying to draw an unbound VOA\n";
  }
  glDrawElements(m_mode,static_cast<GLsizei>(m_indicesCount),m_indexType,static_cast<ngl::Real *>(nullptr));
}


void MultiBufferIndexVAO::draw(int _startIndex, int _amount) const
{
  if(m_allocated == false)
  {
    std::cerr<<"Warning trying to draw an unallocated VOA\n";
  }
  if(m_bound == false)
  {
    std::cerr<<"Warning trying to draw an unbound VOA\n";
  }

  switch(m_indexType)
  {
    case GL_UNSIGNED_INT   :
     glDrawElements(m_mode,static_cast<GLsizei>(_amount),m_indexType,static_cast<GLuint *>(nullptr)+_startIndex);
    break;
    case GL_UNSIGNED_SHORT :
      glDrawElements(m_mode,static_cast<GLsizei>(_amount),m_indexType,static_cast<GLushort *>(nullptr)+_startIndex);
    break;
    case GL_UNSIGNED_BYTE :
      glDrawElements(m_mode,static_cast<GLsizei>(_amount),m_indexType,static_cast<GLubyte *>(nullptr)+_startIndex);
    break;
    default : std::cerr<<"wrong data type send for index value\n"; break;
  }
}

ngl::Real * MultiBufferIndexVAO::mapBuffer(unsigned int _index, GLenum _accessMode)
{
    ngl::Real *ptr=nullptr;
    bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
    ptr = static_cast<ngl::Real *>(glMapBuffer(GL_ARRAY_BUFFER, _accessMode));
    return ptr;
  }

void MultiBufferIndexVAO::removeVAO()
{
  if(m_bound == true)
  {
    unbind();
  }
  if( m_allocated ==true)
  {
      glDeleteBuffers(1,&m_buffer);
  }
  glDeleteVertexArrays(1,&m_id);
  m_allocated=false;
}


//void MultiBufferIndexVAO::setData(size_t _size, const GLfloat &_data, GLenum _mode)
void MultiBufferIndexVAO::setData(const VertexData &_data)
{

  if(m_bound == false)
  {
  std::cerr<<"trying to set VOA data when unbound\n";
  }
  GLuint vboID;
  glGenBuffers(1, &vboID);

  // now we will bind an array buffer to the first one and load the data for the verts
  glBindBuffer(GL_ARRAY_BUFFER, vboID);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(_data.m_size), &_data.m_data, _data.m_mode);

  m_allocated=true;
}

void MultiBufferIndexVAO::setIndices(unsigned int _indexSize,const GLvoid *_indexData,GLenum _indexType,GLenum _mode)
{
  GLuint iboID;
  glGenBuffers(1, &iboID);
  // we need to determine the size of the data type before we set it
  // in default to a ushort
  int size=sizeof(GLushort);
  switch(_indexType)
  {
    case GL_UNSIGNED_INT   : size=sizeof(GLuint);   break;
    case GL_UNSIGNED_SHORT : size=sizeof(GLushort); break;
    case GL_UNSIGNED_BYTE  : size=sizeof(GLubyte);  break;
    default : std::cerr<<"wrong data type send for index value\n"; break;
  }
  // now for the indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indexSize * static_cast<GLsizeiptr>(size), const_cast<GLvoid *>(_indexData), _mode);
  m_indexType=_indexType;
}


