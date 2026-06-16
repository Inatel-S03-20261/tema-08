#pragma once

class IPublicadorResultado {
 public:
  virtual ~IPublicadorResultado() {}
  virtual bool publicar(const char* payload) = 0;
};
