# CStreetMap

`CStreetMap` is an abstract interface that represents a simplified OpenStreetMap-style street map.

It exposes two main object types:

- **Nodes** (`CStreetMap::SNode`): point locations (latitude/longitude) with string attributes.
- **Ways** (`CStreetMap::SWay`): ordered lists of node IDs (a polyline) with string attributes.

The interface provides two access patterns for both nodes and ways:

1. **By index** (in insertion / file order)
2. **By ID** (OSM numeric ID)

## Types

- `TNodeID`, `TWayID`: 64-bit unsigned identifiers.
- `TLocation`: `std::pair<double,double>` holding `(latitude, longitude)`.
- `InvalidNodeID`, `InvalidWayID`: sentinel invalid values.

## Node (`SNode`) API

- `ID()`: returns the node ID.
- `Location()`: returns `(lat, lon)`.
- Attribute functions:
  - `AttributeCount()`
  - `GetAttributeKey(index)` (returns `""` if out of range)
  - `HasAttribute(key)`
  - `GetAttribute(key)` (returns `""` if missing)

## Way (`SWay`) API

- `ID()`: returns the way ID.
- `NodeCount()`: number of node references.
- `GetNodeID(index)`: returns the referenced node ID or `InvalidNodeID` if out of range.
- Attribute functions:
  - `AttributeCount()`
  - `GetAttributeKey(index)` (returns `""` if out of range)
  - `HasAttribute(key)`
  - `GetAttribute(key)` (returns `""` if missing)

## Map-level API

- `NodeCount()` / `WayCount()`
- `NodeByIndex(index)` / `WayByIndex(index)`
- `NodeByID(id)` / `WayByID(id)`

For lookups, out-of-range indices or missing IDs should return `nullptr`.