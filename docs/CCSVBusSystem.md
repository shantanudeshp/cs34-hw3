# CCSVBusSystem

Concrete implementation of `CBusSystem` that loads stop and route data from two CSV files via `CDSVReader`.

## Constructor

```cpp
CCSVBusSystem(std::shared_ptr<CDSVReader> stopsrc, std::shared_ptr<CDSVReader> routesrc);
```

- `stopsrc` — DSV reader for the stops CSV. Expected format:
  ```
  stop_id,node_id
  22043,2849810514
  22358,2849805223
  ...
  ```
- `routesrc` — DSV reader for the routes CSV. Expected format:
  ```
  route,stop_id
  A,22258
  A,22169
  B,22043
  ...
  ```

The first row of each file is treated as a header and skipped. Multiple rows with the same route name are grouped into a single route, preserving the order they appear in the file.

## Methods

Implements all methods of the `CBusSystem` interface. See [CBusSystem.md](CBusSystem.md) for full documentation.

| Method | Behavior |
|--------|----------|
| `StopCount()` | Number of data rows in stops CSV |
| `RouteCount()` | Number of distinct route names in routes CSV |
| `StopByIndex(i)` | Stop at position `i` in file order |
| `StopByID(id)` | O(1) hash map lookup by stop ID |
| `RouteByIndex(i)` | Route at position `i` in first-seen order |
| `RouteByName(name)` | O(1) hash map lookup by route name |

## Example

```cpp
#include "CSVBusSystem.h"
#include "DSVReader.h"
#include "StringDataSource.h"

std::string stopsData = "stop_id,node_id\n1,100\n2,200\n";
std::string routesData = "route,stop_id\nA,1\nA,2\n";

CCSVBusSystem system(
    std::make_shared<CDSVReader>(std::make_shared<CStringDataSource>(stopsData), ','),
    std::make_shared<CDSVReader>(std::make_shared<CStringDataSource>(routesData), ',')
);

std::cout << system.StopCount() << "\n";   // 2
std::cout << system.RouteCount() << "\n";  // 1

auto stop = system.StopByID(1);
std::cout << stop->NodeID() << "\n";       // 100

auto route = system.RouteByName("A");
std::cout << route->StopCount() << "\n";   // 2
std::cout << route->GetStopID(0) << "\n";  // 1
```

## Implementation Notes

- Uses the pimpl pattern: all state lives in a private `SImplementation` struct.
- `SStop` and `SRoute` are concrete inner structs defined inside `SImplementation`.
- Stop IDs and node IDs are parsed with `std::stoull` to handle the full `uint64_t` range.
- Stops are stored in a `vector` for index access and an `unordered_map` for O(1) ID lookup.
- Routes are stored similarly, grouped by name with stops in insertion order.
