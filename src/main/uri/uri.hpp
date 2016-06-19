/*
 * Copyright 2016 Joe Eaves.
 * This file has been modified from it's original source, found here:
 *   https://raw.githubusercontent.com/facebook/folly/master/folly/Uri.h
 *
 * Modifications are to de-facebook-ify the code, so I don't need the whole of Folly.
 * The original copyright notice is included below for compliance.
 */
/* Copyright 2016 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string>
#include <map>
#include <list>
#include <regex>
#include <sstream>

#include "main/exceptions/uri.hpp"

using namespace std;
using namespace cppm::exceptions::uri;


namespace cppm { namespace messages {

static const regex uriRegex(
	"([a-zA-Z][a-zA-Z0-9+.-]*):"  // scheme:
	"([^?#]*)"                    // authority and path
	"(?:\\?([^#]*))?"             // ?query
	"(?:#(.*))?");                // #fragment
static const regex authorityAndPathRegex("//([^/]*)(/.*)?");
static const regex authorityRegex(
	"(?:([^@:]*)(?::([^@]*))?@)?"  // username, password
	"(\\[[^\\]]*\\]|[^\\[:]*)"     // host (IP-literal (e.g. '['+IPv6+']',
								   // dotted-IPv4, or named host)
	"(?::(\\d*))?");               // port
static const regex queryParamRegex(
	"(^|&)" /*start of query or start of parameter "&"*/
	"([^=&]*)=?" /*parameter name and "=" if value is expected*/
	"([^=&]*)" /*parameter value*/
	"(?=(&|$))" /*forward reference, next should be end of query or start of next parameter*/);

/**
 * Class representing a URI.
 *
 * Consider http://www.facebook.com/foo/bar?key=foo#anchor
 *
 * The URI is broken down into its parts: scheme ("http"), authority
 * (ie. host and port, in most cases: "www.facebook.com"), path
 * ("/foo/bar"), query ("key=foo") and fragment ("anchor").  The scheme is
 * lower-cased.
 *
 * If this Uri represents a URL, note that, to prevent ambiguity, the component
 * parts are NOT percent-decoded; you should do this yourself with
 * uriUnescape() (for the authority and path) and uriUnescape(...,
 * UriEscapeMode::QUERY) (for the query, but probably only after splitting at
 * '&' to identify the individual parameters).
 */
class Uri {
 public:
  /**
   * Parse a Uri from a string.  Throws std::invalid_argument on parse error.
   */
  Uri(){};
  Uri(string str);
  void parseUri(const string& str);


  const string& scheme() const { return scheme_; }
  const string& scheme(const string& newscheme) { scheme_ = newscheme; return scheme(); }
  const string& username() const { return username_; }
  const string& password() const { return password_; }
  /**
   * Get host part of URI. If host is an IPv6 address, square brackets will be
   * returned, for example: "[::1]".
   */
  const string& host() const { return host_; }
  const string& host(const string& newhost) { hasAuthority_ = true; host_ = newhost; return host(); }
  /**
   * Get host part of URI. If host is an IPv6 address, square brackets will not
   * be returned, for exmaple "::1"; otherwise it returns the same thing as
   * host().
   *
   * hostname() is what one needs to call if passing the host to any other tool
   * or API that connects to that host/port; e.g. getaddrinfo() only understands
   * IPv6 host without square brackets
   */
  string hostname() const;
  uint16_t port() const { return port_; }
  const string& path() const { return path_; }
  const string& path(const string& newpath) { path_ = newpath; return path(); };
  const string& query() const { return query_; }
  const string& query(const string& newquery) { query_ = newquery; return query(); };
  const string& fragment() const { return fragment_; }
  const string& fragment(const string& newfragment) { fragment_ = newfragment; return fragment(); };

  string authority() const;

  string toString() const;
  string str() const { return toString(); }

  void setPort(uint16_t port) {
    hasAuthority_ = true;
    port_ = port;
  }

  /**
   * Get query parameters as key-value pairs.
   * e.g. for URI containing query string:  key1=foo&key2=&key3&=bar&=bar=
   * In returned list, there are 3 entries:
   *     "key1" => "foo"
   *     "key2" => ""
   *     "key3" => ""
   * Parts "=bar" and "=bar=" are ignored, as they are not valid query
   * parameters. "=bar" is missing parameter name, while "=bar=" has more than
   * one equal signs, we don't know which one is the delimiter for key and
   * value.
   *
   * Note, this method is not thread safe, it might update internal state, but
   * only the first call to this method update the state. After the first call
   * is finished, subsequent calls to this method are thread safe.
   *
   * @return  query parameter key-value pairs in a vector, each element is a
   *          pair of which the first element is parameter name and the second
   *          one is parameter value
   */
  bool hasParam(const string& key) const;
  const map<string, list<string>>& getQueryParams() const;
  const list<string>& getQueryParam(const string& key) const;
  void setQueryParams(const map<string, list<string>>& newParams);
  void addQueryParam(const pair<string, list<string>>& newParams, bool addToQuery = false);
  void addQueryParam(const string& key, const string& value, bool addToQuery = false);

 private:
  string scheme_ = "global";
  string username_;
  string password_;
  string host_ = "NULL";
  bool hasAuthority_ = true;
  uint16_t port_ = 0;
  string path_;
  string query_;
  string fragment_;
  map<string, list<string>> queryParams_;

  void parseQuery();
};

}}
