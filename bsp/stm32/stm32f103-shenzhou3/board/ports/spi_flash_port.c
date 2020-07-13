#include <rtthread.h>

#ifdef BSP_USING_SPI_FLASH
#include <drv_spi.h>
#include <spi_flash.h>
#include <spi_flash_sfud.h>
#include <dfs_fs.h>

int spi_flash_init(void)
{
	int result = rt_hw_spi_device_attach("spi1", "spi10", GPIOC, GPIO_PIN_4);

    return result;
	
}
INIT_DEVICE_EXPORT(spi_flash_init);

int spi_flash_mount(void)
{
	static rt_spi_flash_device_t rtt_dev = NULL;
	rtt_dev = rt_sfud_flash_probe("flash0", "spi10");
	
	if (rtt_dev==NULL)
		return RT_ERROR;
	

//	
//	if (dfs_mount("flash0", "/", "elm", 0, 0) == RT_EOK)
//	{
//		rt_kprintf("flash0 mount to '/'\n");
//		return RT_EOK;
//	}
//	else
//	{
//		rt_kprintf("flash0 mount to '/' failed!\n");
//		return RT_ERROR;
//	}
//	
}
INIT_APP_EXPORT(spi_flash_mount);
#endif /*BSP_USING_SPI_FLASH*/
