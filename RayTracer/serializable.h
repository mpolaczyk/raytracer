#pragma once

template<class T>
class serializable
{
  virtual T serialize() = 0;
  virtual void deserialize(const T& payload) = 0;
};