#include <rtthread.h>

#ifdef BSP_USING_SPI_FLASH
#include <drv_spi.h>

int spi_flash_init(void)
{
	int result = rt_hw_spi_device_attach("spi1", "spi10", GPIOC, GPIO_PIN_4);

    return result;
	
}
INIT_DEVICE_EXPORT(spi_flash_init);
#endif /*BSP_USING_SPI_FLASH*/
