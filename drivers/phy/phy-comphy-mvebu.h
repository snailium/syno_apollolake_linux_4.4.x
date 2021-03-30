#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#ifndef _COMPHY_MVEBU_H
#define _COMPHY_MVEBU_H

#define MVEBU_COMPHY_MAX_CNT	6
#define MVEBU_COMPHY_FUNC_MAX	11

#define to_mvebu_comphy_priv(lane) \
	container_of((lane), struct mvebu_comphy_priv, lanes[(lane)->index])

#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_10)
enum {
	COMPHY_LANE0 = 0,
	COMPHY_LANE1,
	COMPHY_LANE2,
	COMPHY_LANE3,
	COMPHY_LANE4,
	COMPHY_LANE5,
	COMPHY_LANE_MAX,
};

#endif /* CONFIG_SYNO_LSP_ARMADA_17_10 */
enum reg_width_type {
	REG_16BIT = 0,
	REG_32BIT,
};

#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
struct mvebu_comphy_priv {
	struct device *dev;
	void __iomem *comphy_regs;
	void __iomem *comphy_pipe_regs;
	spinlock_t lock;
	const struct mvebu_comphy_soc_info *soc_info;
	struct mvebu_comphy {
		struct phy *phy;
		int mode;
		int index;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_10)
		/* in case PCIe is in End point mode */
		bool skip_pcie_power_off;
		struct misc_info {
			int pcie_width: 8;
			int pcie_is_ep: 1;
		} misc;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_10 */
	} lanes[MVEBU_COMPHY_MAX_CNT];
};

struct mvebu_comphy_soc_info {
	int num_of_lanes;
	int functions[MVEBU_COMPHY_MAX_CNT][MVEBU_COMPHY_FUNC_MAX];
	struct phy_ops *comphy_ops;
};

static inline void __maybe_unused reg_set(void __iomem *addr, u32 data, u32 mask)
{
	u32 reg_data;

	reg_data = readl(addr);
	reg_data &= ~mask;
	reg_data |= data;
	writel(reg_data, addr);
}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
static inline void __maybe_unused reg_set16(void __iomem *addr, u16 data, u16 mask)
{
	u16 reg_data;

	reg_data = readw(addr);
	reg_data &= ~mask;
	reg_data |= data;
	writew(reg_data, addr);
}

#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
static inline u32 __maybe_unused polling_with_timeout(void __iomem *addr,
						      u32 val,
						      u32 mask,
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
						      unsigned long usec_timout,
						      enum reg_width_type type)
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
						      unsigned long usec_timout)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
{
	u32 data;

	do {
		udelay(1);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		if (type == REG_16BIT)
			data = readw(addr) & mask;
		else
			data = readl(addr) & mask;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		data = readl(addr) & mask;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	} while (data != val  && --usec_timout > 0);

	if (usec_timout == 0)
		return data;

	return 0;
}

/* Function declaration */
int mvebu_comphy_set_mode(struct phy *phy, enum phy_mode mode);
enum phy_mode mvebu_comphy_get_mode(struct phy *phy);

#endif /* _COMPHY_MVEBU_H */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
