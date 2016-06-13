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
		if (!regex_match(authority.first, authority.second,	authorityMatch,	authorityRegex)) {
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

	parseQuery();
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

void Uri::parseQuery() {
	if (!query_.empty()) {
		queryParams_.clear();

		cregex_iterator paramBeginItr(query_.data(),
									  query_.data() + query_.size(), queryParamRegex);
		cregex_iterator paramEndItr;

		for (auto itr = paramBeginItr; itr != paramEndItr; itr++) {
			if (itr->length(2) == 0) {
				// key is empty, ignore it
				continue;
			}

			addQueryParam(
				string((*itr)[2].first, (*itr)[2].second),
				string((*itr)[3].first, (*itr)[3].second)
			);
		}
	} else {
		throw GeneralFailure("parseQuery found query_ empty!");
	}
}

bool Uri::hasParam(const string& key) {
	return queryParams_.find(key) != queryParams_.end();
}

const map<string, list<string>>& Uri::getQueryParams() {
  return queryParams_;
}

const list<string>& Uri::getQueryParam(const string& key) {
	if (hasParam(key)) {
		return queryParams_.at(key);
	}

	throw ParamNotFound(key);
}

void Uri::setQueryParams(const map<string, list<string>>& newParams) {
	queryParams_.clear();
	query_.clear();
	for (auto& param : newParams) {
		addQueryParam(param, true);
	}
}

void Uri::addQueryParam(const pair<string, list<string>>& newParams, bool addToQuery) {
	for (auto& paramVal : newParams.second)
		addQueryParam(newParams.first, paramVal, addToQuery);
}

void Uri::addQueryParam(const string& key, const string& value, bool addToQuery) {
	if (hasParam(key)) {
		queryParams_[key] = { value };
	} else {
		queryParams_[key].emplace_back(value);
	}

	if (addToQuery) {
		if (queryParams_.size() > 1 && ! query_.empty())
			query_ += "&";
		query_ += key + "=" + value;
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
