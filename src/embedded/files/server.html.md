# `server.html`

## Overview

- Uses Tailwind CLI for styling
- Template for `CPP Crow mustache`
- Serves as a frontend the to the Server for logging purposes
- Supports Pagination
- Allows Filtering logs by provided loggers
- Dark and Light Mode works

### C++ SIDE

- Following info is provide by Crow Mustache Templater:
  - `port`: the port of websocket connection
  - `refreshInterval`: how often should the page refresh; defaults to 50 ms
  - `m_loggers`: list of logger_ids and names to generate chips.
- Data is sent in binary format, refer to [`Server.hpp Server::encodeLogMessage`](../../src/Server.hpp#L91)

### VARIABLES

- `messageCache`: `Message` Object keyed by `loggerId`; limits per logger to 10,000 messages to prevent memory bloat.
- `mergedCache`: Flattened + timestamp-sorted view of selected loggers.
- `PAGE_SIZE`: Number of logs per page.
- `selectedLoggers`: Set of currently selected loggers for filtering
- `messageQueue`: Queue to batch incoming WebSocket messages before insertion

### MAIN LOOP

- #### INIT

  - Logger chips are auto-generated from `m_loggers` provided.
  - On Page Load opens a websocket connection at `ws://localhost:{{port}}/ws`
  - Theme is initialized from `localStorage` (Dark/Light)

- #### `refreshInterval` CYCLE

  - Recieved messages are stored as `Message` structure in the cache: `messageCache`
  - `addMessage` is called which in turns calls `insertIntoMerged`
    - `insertIntoMerged` adds the message log to its respective place; uses `binary search` algorithm
  - Stats are updated
  - Pagination Processing Happens; Loads only 200 messages per page

- #### EVENTS

  - **MESSAGE ARRIVAL**: `ws.onmessage`:
    - When Message arrives as binary, it is decoded and sent to queue `messageQueue`
  - **LOG CHIPS CLICK CALLBACK**: `loggerChips.addEventListener("click", e =>`:
    - Adds/removes logger IDs to/from `selectedLoggers`
    - Updates chips UI (`updateChips`)
    - Rebuilds `mergedCache` and rerenders current page
  - **CLEAR LOGS**: `clearBtn.onclick`:
    - Clears all cache
    - Clears queue
    - Updates stats
    - Rerenders by calling `renderSelectedMessages`
  - **PAGINATION BUTTONS**: `prevBtn.addEventListener("click", () =>` and `nextBtn.addEventListener("click", () =>`:
    - Changes page counter
    - Rerenders by calling `renderSelectedMessages`

### COLOR & LOG LEVEL HANDLING

- `Message.computeColors` defines **Tailwind CSS classes** for log level backgrounds, text, and hover states
- Predefined log levels: `INFO`, `WARN`, `ERROR`
- Unknown log levels generate a deterministic HSL color based on hash

### PERFORMANCE & QUEUE BATCHING

- `messageQueue` batches incoming WebSocket messages to **avoid excessive DOM updates**
- Batched messages are inserted incrementally into `mergedCache` for performance
- `mergedCache` is rebuilt only when selected loggers change, not every batch flush

### THEME HANDLING

- Dark/Light mode toggle persists in `localStorage.theme`
- Uses Tailwind’s `class="dark"` strategy
- Toggle button updates UI immediately without page reload

### BENCHMARK

- Machine: Macbook M2 Air: 8GB unified memoory; 256GB Storage
- Handles 10k messages sent in >100ms with ease.
