#pragma once
struct Curve;

// Returns the signed fraction (upper area - lower area) / total area.
double AO(const Curve& curve);
