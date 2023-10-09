#include "LayerStack.hpp"

namespace TE {

LayerStack::LayerStack() { layerInsert = 0; }

LayerStack::~LayerStack() {
  for (Layer* layer : layers) {
    layer->onDetach();
    delete layer;
  }
}

void LayerStack::pushLayer(Layer* layer) {
  layers.insert(layers.begin() + layerInsert, layer);
  layerInsert++;

  layer->onAttach();
}

void LayerStack::popLayer(Layer* layer) {
  auto it = std::find(layers.begin(), layers.begin() + layerInsert, layer);
  if (it != layers.begin() + layerInsert) {
    layer->onDetach();
    layers.erase(it);
    layerInsert--;
  }
}

void LayerStack::pushOverlay(Layer* layer) {
  layers.push_back(layer);
  layer->onAttach();
}

void LayerStack::popOverlay(Layer* layer) {
  auto it = std::find(layers.begin() + layerInsert, layers.end(), layer);
  if (it != layers.end()) {
    layer->onDetach();
    layers.erase(it);
  }
}
}  // namespace TE