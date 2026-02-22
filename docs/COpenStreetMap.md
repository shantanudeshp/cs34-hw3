# COpenStreetMap

`COpenStreetMap` is a concrete implementation of the `CStreetMap` interface.

It builds a street map by reading an OpenStreetMap (OSM) XML stream using the provided `CXMLReader`.

## Input format supported

This implementation focuses on the subset required for the assignment:

- **`<node ...>`** elements
  - Reads attributes: `id`, `lat`, `lon`
  - Reads child tags: `<tag k="..." v="..."/>`

- **`<way ...>`** elements
  - Reads attribute: `id`
  - Reads child node refs: `<nd ref="..."/>`
  - Reads child tags: `<tag k="..." v="..."/>`

All other XML elements are ignored.

## Data organization

To support both access patterns efficiently:

- Nodes are stored in a vector in insertion order.
- A hash map from node ID to vector index enables O(1) `NodeByID`.

Similarly for ways:

- Ways are stored in a vector in insertion order.
- A hash map from way ID to vector index enables O(1) `WayByID`.

Each node/way stores attributes in:

- A vector of `(key,value)` pairs to preserve insertion order for `GetAttributeKey(index)`.
- A hash map from key to value for fast `HasAttribute` / `GetAttribute`.

## Error / edge behavior

- Missing IDs or out-of-range indices return `nullptr` for object lookup.
- Out-of-range attribute key index returns `""`.
- Missing attribute key returns `""`.
- Out-of-range way node index returns `CStreetMap::InvalidNodeID`.