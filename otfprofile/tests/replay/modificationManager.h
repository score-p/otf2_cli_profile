#include <stdint.h>

#ifndef MOD_MANAGER_H
#define MOD_MANAGER_H

typedef struct struct_ModOptions{
	float dsize_factor;
	float wtime_factor;
	struct_ModOptions():dsize_factor(1),wtime_factor(1){ }
	struct_ModOptions(const struct_ModOptions& opts) : dsize_factor(opts.dsize_factor), wtime_factor(opts.wtime_factor) { }
	struct_ModOptions(float dsize, float wtime): dsize_factor(dsize), wtime_factor(wtime) { }
	struct_ModOptions& operator=(const struct_ModOptions& opts)
	{
		if (this == &opts) return *this;
		dsize_factor = opts.dsize_factor;
		wtime_factor = opts.wtime_factor;
		return *this;
	}
} ModOptions;

class ModManager
{
	private:
		ModOptions options;
	
	public:
		ModManager(const ModOptions& options);
		ModManager(const ModManager& manager);
		ModManager& operator=(const ModManager& manager);
		uint32_t dsize(uint32_t osize) const;
		uint32_t wtime(uint32_t wtime) const;
		float get_dsize_factor() const { return options.dsize_factor; }
		float get_wtime_factor() const { return options.wtime_factor; }
};

#endif /* MOD_MANAGER_H*/
