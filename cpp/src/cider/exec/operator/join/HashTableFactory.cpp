/*
 * Copyright (c) 2022 Intel Corporation.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

namespace cider_hashtable {
template <typename Key,
          typename Value,
          typename Hash,
          typename KeyEqual,
          typename Grower,
          typename Allocator>
template <typename... Args>
std::unique_ptr<BaseHashTable<Key, Value, Hash, KeyEqual, Grower, Allocator>>
HashTableSelector<Key, Value, Hash, KeyEqual, Grower, Allocator>::createForJoin(
    HashTableType hashtable_type,
    Args&&... args) {
  switch (hashtable_type) {
    case LINEAR_PROBING:
      return std::make_unique<LinearProbeHashTable<Key, Value, Hash, KeyEqual>>(
          std::forward<Args>(args)...);
    default:
      return std::make_unique<LinearProbeHashTable<Key, Value, Hash, KeyEqual>>(
          std::forward<Args>(args)...);
  }
}
}  // namespace cider_hashtable
