/*
config
*/

#pragma once

class Config {
public:
	Config();
	~Config();

	void						Load(const char *res_dir);

	int							GetAsInteger(const char *key, int default_) const;
	float						GetAsFloat(const char *key, float default_) const;
	const char *				GetAsString(const char *key, const char *default_) const;
	vec3						GetAsVec3(const char *key, const vec3 &default_) const;

private:

	struct key_value_s {
		const char *			mKey;
		const char *			mValue;
		key_value_s *			mNext;

		key_value_s() {
			mKey = mValue = nullptr;
			mNext = nullptr;
		}
	};

	key_value_s *				mPairs;
	char						mBuffer[4096];

	key_value_s *				FindPair(const char *key) const;
};
