#pragma once

#include <cstddef>

#include "Layer.hpp"
#include "tepch.hpp"

namespace TE {

class LayerStack {
 public:
  LayerStack();
  ~LayerStack();

  void pushLayer(Layer* layer);
  void popLayer(Layer* layer);
  void pushOverlay(Layer* layer);
  void popOverlay(Layer* layer);

  std::vector<Layer*>::iterator begin() { return layers.begin(); }
  std::vector<Layer*>::iterator end() { return layers.end(); }
  std::vector<Layer*>::reverse_iterator rbegin() { return layers.rbegin(); }
  std::vector<Layer*>::reverse_iterator rend() { return layers.rend(); }

  std::vector<Layer*>::const_iterator begin() const { return layers.begin(); }
  std::vector<Layer*>::const_iterator end() const { return layers.end(); }
  std::vector<Layer*>::const_reverse_iterator rbegin() const {
    return layers.rbegin();
  }
  std::vector<Layer*>::const_reverse_iterator rend() const {
    return layers.rend();
  }

 private:
  std::vector<Layer*> layers;
  size_t layerInsert;
};

}  // namespace TE