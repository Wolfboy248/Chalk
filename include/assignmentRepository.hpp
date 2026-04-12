#pragma once

#include "types.hpp"

class AssignmentRepository {
public:
  static void save(const Assignment& assignment, const QString& path);
  static Assignment load(const QString& path);
};

