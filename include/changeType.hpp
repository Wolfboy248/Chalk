#pragma once

enum class ChangeType {
  Structure, // Tasks added, removed or reordered -> full re-render
  Content,
  Metadata,
  Selection, // Active task changed -> dont render
};

