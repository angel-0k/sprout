/**
 * @file associated_uris.cpp Implementation of the AssociatedURIs class.
 *
 * Copyright (C) Metaswitch Networks
 * If license terms are provided to you in a COPYING file in the root directory
 * of the source code repository by which you are accessing this code, then
 * the license outlined in that COPYING file applies to your use.
 * Otherwise no rights are granted except for those provided to you by
 * Metaswitch Networks in a separate written agreement.
 */

#include "associated_uris.h"
#include "log.h"

#include <algorithm>

// Gets the default URI. We return the first unbarred URI. If there is no
// unbarred URI, we don't return anything unless it is an emergency in which
// case we return the first URI.
bool AssociatedURIs::get_default_impu(std::string& uri,
                                      bool emergency)
{
  std::vector<std::string> unbarred_uris = this->get_unbarred_uris();
  if (!unbarred_uris.empty())
  {
    uri = unbarred_uris.front();
    return true;
  }

  if ((emergency) &&
      (!_associated_uris.empty()))
  {
    uri = _associated_uris.front();
    return true;
  }

  return false;
}

// Checks if the URI is in the list of associated URIs.
bool AssociatedURIs::contains_uri(std::string uri)
{
  return (std::find(_associated_uris.begin(), _associated_uris.end(), uri) !=
          _associated_uris.end());
}

// Adds a URI and its barring state to the list of associated URIs.
void AssociatedURIs::add_uri(std::string uri,
                             bool barred)
{
  _associated_uris.push_back(uri);
  add_barring_status(uri, barred);
}

// Adds the barring state of a URI. This map includes URIs that aren't in the
// associated URI list (e.g. it includes non-distinct IMPUs)
void AssociatedURIs::add_barring_status(std::string uri,
                                        bool barred)
{
  _barred_map[uri] = barred;
}

// Removes all URIs.
void AssociatedURIs::clear_uris()
{
  _associated_uris.clear();
  _barred_map.clear();
}

// Returns if the specified URI is barred.
bool AssociatedURIs::is_impu_barred(std::string uri)
{
  // Sometimes we don't have the barring status of the specific URI as it's
  // actually an URI that matches a wildcard - get the wildcard URI before
  // we check the barring status. In the case where a non-distinct IMPU
  // has had its barring indication set specifically in the IMS subscription
  // we got from the HSS then it was directly added to the _barred_map (and
  // not added to the _distinct_to_wildcard map).
  std::string uri_to_check = uri;
  if (_distinct_to_wildcard.find(uri) != _distinct_to_wildcard.end())
  {
    uri_to_check = _distinct_to_wildcard[uri];
  }

  if (_barred_map.find(uri_to_check) != _barred_map.end())
  {
    return _barred_map[uri_to_check];
  }
  else
  {
    // We shouldn't ever end up here - return false (we do hit this in UTs
    // though as we don't always use valid data).
    TRC_DEBUG("No barring information available for %s", uri.c_str());
    return false;
  }
}

// Returns all unbarred associated URIs.
std::vector<std::string> AssociatedURIs::get_unbarred_uris()
{
  std::vector<std::string> unbarred_uris;

  for (std::string uri : _associated_uris)
  {
    if (!_barred_map[uri])
    {
      unbarred_uris.push_back(uri);
    }
  }

  return unbarred_uris;
}

// Returns all barred associated URIs.
std::vector<std::string> AssociatedURIs::get_barred_uris()
{
  std::vector<std::string> barred_uris;

  for (std::string uri : _associated_uris)
  {
    if (_barred_map[uri])
    {
      barred_uris.push_back(uri);
    }
  }

  return barred_uris;
}

// Returns all associated URIs.
std::vector<std::string> AssociatedURIs::get_all_uris()
{
  return _associated_uris;
}

// Sets up the link between a distinct IMPU and its wildcard.
void AssociatedURIs::add_wildcard_mapping(std::string wildcard,
                                          std::string distinct)
{
  _distinct_to_wildcard.insert(std::make_pair(distinct, wildcard));
}
