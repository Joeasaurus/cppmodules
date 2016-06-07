/*
 * Copyright 2016 Joe Eaves.
 * This file has been modified from it's original source, found here:
 *   https://raw.githubusercontent.com/facebook/folly/master/folly/Uri.cpp
 * and here
 *   https://github.com/facebook/folly/blob/master/folly/Uri-inl.h
 *
 * Modifications are to de-facebook-ify the code, so I don't need the whole of Folly.
 * The original copyright notice is included below for compliance.
 */
/*
 * Copyright 2016 Facebook, Inc.
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

#include "main/messages/uri.hpp"

using namespace std;

namespace cppm { namespace messages {

string submatch(const cmatch& m, size_t idx) {
  auto& sub = m[idx];
  return string(sub.first, sub.second);
}

void toLower(string& s) {
  for (auto& c : s) {
    c = tolower(c);
  }
}

Uri::Uri(string str) {
	parseUri(str);
}

void Uri::parseUri(const string& str) {
	cmatch match;
    if (!regex_match(str.c_str(), match, uriRegex)) {
      throw std::invalid_argument("invalid URI " + str);
    }

    scheme_ = submatch(match, 1);
    toLower(scheme_);

    string authorityAndPath(match[2].first, match[2].second);
    cmatch authorityAndPathMatch;
    if (!regex_match(authorityAndPath.c_str(), authorityAndPathMatch, authorityAndPathRegex)) {
      // Does not start with //, doesn't have authority
      hasAuthority_ = false;
      path_ = authorityAndPath;
    } else {


      auto authority = authorityAndPathMatch[1];
      cmatch authorityMatch;
      if (!regex_match(authority.first,
                              authority.second,
                              authorityMatch,
                              authorityRegex)) {
        throw std::invalid_argument("invalid URI authority " + string(authority.first, authority.second));
      }

      string port(authorityMatch[4].first, authorityMatch[4].second);
      if (!port.empty()) {
        port_ = static_cast<short>(std::stoi(port)); //ugck
      }

      hasAuthority_ = true;
      username_ = submatch(authorityMatch, 1);
      password_ = submatch(authorityMatch, 2);
      host_ = submatch(authorityMatch, 3);
      path_ = submatch(authorityAndPathMatch, 2);
    }

    query_ = submatch(match, 3);
    fragment_ = submatch(match, 4);
}

string Uri::authority() const {
  string result;

  // Port is 5 characters max and we have up to 3 delimiters.
  result.reserve(host().size() + username().size() + password().size() + 8);

  if (!username().empty() || !password().empty()) {
    result.append(username());

    if (!password().empty()) {
      result.push_back(':');
      result.append(password());
    }

    result.push_back('@');
  }

  result.append(host());

  if (port() != 0) {
    result.push_back(':');
    result.push_back(port());
  }

  return result;
}

string Uri::hostname() const {
  if (host_.size() > 0 && host_[0] == '[') {
    // If it starts with '[', then it should end with ']', this is ensured by
    // regex
    return host_.substr(1, host_.size() - 2);
  }
  return host_;
}

const std::vector<std::pair<string, string>>& Uri::getQueryParams() {
  if (!query_.empty() && queryParams_.empty()) {
    // Parse query string

    cregex_iterator paramBeginItr(
        query_.data(), query_.data() + query_.size(), queryParamRegex);
    cregex_iterator paramEndItr;
    for (auto itr = paramBeginItr; itr != paramEndItr; itr++) {
      if (itr->length(2) == 0) {
        // key is empty, ignore it
        continue;
      }
      queryParams_.emplace_back(
          string((*itr)[2].first, (*itr)[2].second), // parameter name
          string((*itr)[3].first, (*itr)[3].second) // parameter value
          );
    }
  }
  return queryParams_;
}

void Uri::addQueryParam(const pair<string, string>& newParams) {
	queryParams_.push_back(newParams);
	if (queryParams_.size() > 1 && ! query_.empty())
		query_ += "&";
	query_ += newParams.first + "=" + newParams.second;
}

void Uri::setQueryParams(const vector<pair<string, string>>& newParams) {
	queryParams_.clear();
	for (auto& param : newParams) {
		addQueryParam(param);
	}
}

string Uri::toString() const {
  stringstream str;
  if (hasAuthority_) {
    str << scheme_ << "://";
    if (!password_.empty()) {
      str << username_ << ":" << password_ << "@";
    } else if (!username_.empty()) {
      str << username_ << "@";
    }
    str << host_;
    if (port_ != 0) {
      str << ":" << to_string(port_);
    }
  } else {
    str << scheme_ << ":";
  }
  str << path_;
  if (!query_.empty()) {
    str << "?" << query_;
  }
  if (!fragment_.empty()) {
    str << "#" << fragment_;
  }
  return str.str();
}

}}
