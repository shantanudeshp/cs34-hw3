# CBusSystem

Abstract interface for a bus transit system. Provides access to stops and routes loaded from an external data source.

## Types

```cpp
using TStopID = uint64_t;
static const TStopID InvalidStopID; // = std::numeric_limits<uint64_t>::max()
```

## Methods

### StopCount
```cpp
std::size_t StopCount() const noexcept;
```
Returns the total number of stops in the system.

### RouteCount
```cpp
std::size_t RouteCount() const noexcept;
```
Returns the total number of routes in the system.

### StopByIndex
```cpp
std::shared_ptr<SStop> StopByIndex(std::size_t index) const noexcept;
```
Returns the stop at `index`. Returns `nullptr` if `index >= StopCount()`.

### StopByID
```cpp
std::shared_ptr<SStop> StopByID(TStopID id) const noexcept;
```
Returns the stop with the given stop ID. Returns `nullptr` if no stop with that ID exists.

### RouteByIndex
```cpp
std::shared_ptr<SRoute> RouteByIndex(std::size_t index) const noexcept;
```
Returns the route at `index`. Returns `nullptr` if `index >= RouteCount()`.

### RouteByName
```cpp
std::shared_ptr<SRoute> RouteByName(const std::string &name) const noexcept;
```
Returns the route with the given name. Returns `nullptr` if no route with that name exists.

## SStop Interface

Represents a single bus stop.

```cpp
TStopID ID() const noexcept;           // unique stop ID
CStreetMap::TNodeID NodeID() const noexcept; // OSM node ID for this stop's location
```

## SRoute Interface

Represents a bus route as an ordered sequence of stops.

```cpp
std::string Name() const noexcept;                   // route name (e.g. "A")
std::size_t StopCount() const noexcept;              // number of stops on route
TStopID GetStopID(std::size_t index) const noexcept; // stop ID at index, or InvalidStopID
```

## Example

```cpp
// Iterate all stops
for(std::size_t i = 0; i < system.StopCount(); i++){
    auto stop = system.StopByIndex(i);
    std::cout << stop->ID() << " -> node " << stop->NodeID() << "\n";
}

// Look up a route by name and print its stops
auto route = system.RouteByName("A");
if(route){
    for(std::size_t i = 0; i < route->StopCount(); i++){
        std::cout << route->GetStopID(i) << "\n";
    }
}
```
