/**
 * @copyright 2022-present Brian Cairl
 *
 * @file archive_fwd.hpp
 */
#pragma once

namespace sde::serial
{

class hash_archive;
template <typename IArchiveT> class iarchive;
template <typename OArchiveT> class oarchive;
template <typename IStreamT> class binary_iarchive;
template <typename OStreamT> class binary_oarchive;
template <typename IArchiveT> class associative_iarchive;
template <typename OArchiveT> class associative_oarchive;

}  // namespace sde::serial
