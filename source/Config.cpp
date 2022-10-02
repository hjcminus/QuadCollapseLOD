/*
config
*/

#include "Precompiled.h"

Config::Config():mPairs(0) {
}

Config::~Config() {
}

void Config::Load(const char *res_dir) {
	char cfg_file[MAX_PATH];
	sprintf_(cfg_file, "%s%sconfig.cfg", res_dir, PATH_SEPERATOR);

	if (File_LoadText(cfg_file, mBuffer, sizeof(mBuffer)) > 0) {

		char * pc = mBuffer;
		while (*pc) {
			// extract line
			char * line = pc;

			while (*pc && *pc != 13) {
				pc++;
			}

			char line_break = *pc;
			if (*pc) {
				*pc = 0;
			}

			// parse line
			if (*line != 13) { // not a blank line
				char * temp_key = line;
				char * temp_value = strchr(line, '=');
				if (temp_value) {
					*temp_value = 0;

					key_value_s * pair = new key_value_s();

					pair->mKey = temp_key;
					pair->mValue = temp_value + 1;

					if (mPairs) {
						pair->mNext = mPairs;
					}
					mPairs = pair;
				}
			}

			// skip 10
			*pc = line_break;
			if (*pc) {
				*pc = 0;
				pc++;
				while (10 == *pc) {
					pc++;
				}
			}
		}
	}
}

int Config::GetAsInteger(const char *key, int default_) const {
	const key_value_s * p = FindPair(key);
	if (p) {
		return atoi(p->mValue);
	}
	else {
		return default_;
	}
}

float Config::GetAsFloat(const char *key, float default_) const {
	const key_value_s * p = FindPair(key);
	if (p) {
		return (float)atof(p->mValue);
	}
	else {
		return default_;
	}
}

const char * Config::GetAsString(const char *key, const char *default_) const {
	const key_value_s * p = FindPair(key);
	if (p) {
		return p->mValue;
	}
	else {
		return default_;
	}
}

vec3 Config::GetAsVec3(const char *key, const vec3 &default_) const {
	const key_value_s * p = FindPair(key);
	if (p) {

		char buffer[1024] = { 0 };
		strcpy_(buffer, p->mValue);

		float v[3] = { 0.0f };
		int i = 0;
		char * pc = buffer;
		char * sec = pc;
		while (*pc) {
			if (',' == *pc) {
				*pc = 0;
				v[i++] = (float)atof(sec);
				if (i >= 3) {
					break;
				}
				++pc;
				sec = pc;
			}
			else {
				++pc;
			}
		}

		if (i <= 2) {
			v[i] = (float)atof(sec);
		}

		return vec3(v[0], v[1], v[2]);
	}
	else {
		return default_;
	}
}

Config::key_value_s * Config::FindPair(const char *key) const {
	key_value_s * p = mPairs;
	while (p) {
		if (strcmp(key, p->mKey) == 0) {
			return p;
		}
		p = p->mNext;
	}
	return nullptr;
}
