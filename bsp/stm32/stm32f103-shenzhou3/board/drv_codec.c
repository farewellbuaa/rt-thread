#include "drivers/audio.h"

#define DBG_TAG "drv_sound"
#define DBG_LVL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

#include <drv_spi.h>

#include "pcm1770.h"

#define TX_DMA_FIFO_SIZE (2048)


struct stm32_sound
{
	struct rt_audio_device device;
	struct rt_audio_configure replay_config;
	struct rt_spi_device *spi_dev;
	DMA_HandleTypeDef h_dma;
	I2S_HandleTypeDef h_i2s;
	int volume;
	rt_uint8_t *tx_fifo;
};

static struct stm32_sound stm32_snd = {0};

static rt_err_t set_samplerate(int request_samplerate)
{
	int samplerate = I2S_AUDIOFREQ_44K;
	switch(request_samplerate)
	{
		case I2S_AUDIOFREQ_192K:
			samplerate = I2S_AUDIOFREQ_192K;
			break;
		case I2S_AUDIOFREQ_96K:
			samplerate = I2S_AUDIOFREQ_96K;
			break;
		case I2S_AUDIOFREQ_48K:
			samplerate = I2S_AUDIOFREQ_48K;
			break;
		case I2S_AUDIOFREQ_44K:
			samplerate = I2S_AUDIOFREQ_44K;
			break;
		case I2S_AUDIOFREQ_32K:
			samplerate = I2S_AUDIOFREQ_32K;
			break;
		case I2S_AUDIOFREQ_22K:
			samplerate = I2S_AUDIOFREQ_22K;
			break;
		case I2S_AUDIOFREQ_16K:
			samplerate = I2S_AUDIOFREQ_16K;
			break;
		case I2S_AUDIOFREQ_11K:
			samplerate = I2S_AUDIOFREQ_11K;
			break;
		case I2S_AUDIOFREQ_8K:
			samplerate = I2S_AUDIOFREQ_8K;
			break;
		default:
			return -RT_ERROR;
	}
		
	stm32_snd.h_i2s.Init.AudioFreq = samplerate;
	
	if (HAL_I2S_Init(&stm32_snd.h_i2s) != HAL_OK)
	{
		return -RT_ERROR;
	}
	
	return RT_EOK;
}


static rt_err_t set_volume(int volume)
{
	return RT_EOK;
}

static rt_err_t getcaps(struct rt_audio_device *audio, struct rt_audio_caps *caps)
{
	struct stm32_sound *sound = RT_NULL;

	RT_ASSERT(audio != RT_NULL);
	sound = (struct stm32_sound *)audio->parent.user_data;
	
	rt_err_t ret = RT_EOK;
	
	switch(caps->main_type)
	{
		case AUDIO_TYPE_QUERY:
		{
			switch (caps->sub_type)
			{
				case AUDIO_TYPE_QUERY:
					caps->udata.mask = AUDIO_TYPE_OUTPUT | AUDIO_TYPE_MIXER;
					break;
				default:
					ret = -RT_ERROR;
				break;
			}
			break;
		}		
		case AUDIO_TYPE_OUTPUT:
		{
			switch(caps->sub_type)
			{
				case AUDIO_DSP_PARAM:
					caps->udata.config.channels   = sound->replay_config.channels;
					caps->udata.config.samplebits = sound->replay_config.samplebits;
					caps->udata.config.samplerate = sound->replay_config.samplerate;
					break;
				default:
					ret = -RT_ERROR;
					break;
			}
			break;
		}
		case AUDIO_TYPE_MIXER:
		{
			switch (caps->sub_type)
			{
				case AUDIO_MIXER_QUERY:
					caps->udata.mask = AUDIO_MIXER_VOLUME | AUDIO_MIXER_LINE;
					break;
				case AUDIO_MIXER_VOLUME:
					caps->udata.value = sound->volume;
					break;
				case AUDIO_MIXER_LINE:
					break;
				default:
					ret = -RT_ERROR;
					break;
			}
			break;
		}
		default:
			ret = -RT_ERROR;
			break;
	}
	return ret;
}

static rt_err_t configure(struct rt_audio_device *audio, struct rt_audio_caps *caps)
{
	struct stm32_sound *sound = RT_NULL;

	RT_ASSERT(audio != RT_NULL);
	sound = (struct stm32_sound *)audio->parent.user_data; (void)sound;
	
	rt_err_t ret = RT_EOK;
	
	switch(caps->main_type)
	{
		case AUDIO_TYPE_OUTPUT:
		{
			switch(caps->sub_type)
			{
				case AUDIO_DSP_SAMPLERATE:
					ret = set_samplerate(caps->udata.config.samplerate);
					break;
				case AUDIO_DSP_PARAM:
					//caps.udata.config.samplerate	
					//caps.udata.config.channels
					//caps.udata.config.samplebits = 16; 
					ret = set_samplerate(caps->udata.config.samplerate);
					break;
				default:
					ret = -RT_ERROR;
					break;
			}
			break;
		}
		case AUDIO_TYPE_MIXER:
		{
			switch (caps->sub_type)
			{
				case AUDIO_MIXER_VOLUME:
					ret = set_volume(caps->udata.value);				
					break;
				default:
					ret = -RT_ERROR;
					break;
			}
			break;
		}
		default:
			ret = -RT_ERROR;
			break;
	}

	return ret;
}

static rt_err_t init(struct rt_audio_device *audio)
{
	struct stm32_sound *sound = RT_NULL;
	RT_ASSERT(audio != RT_NULL);
	sound = (struct stm32_sound *)audio->parent.user_data;
	
	DMA_HandleTypeDef *dma = &sound->h_dma;
	I2S_HandleTypeDef *i2s = &sound->h_i2s;
	
	i2s->Instance = SPI3;
	i2s->Init.Mode = I2S_MODE_MASTER_TX;
	i2s->Init.Standard = I2S_STANDARD_PHILIPS;
	i2s->Init.DataFormat = I2S_DATAFORMAT_16B;
	i2s->Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
	i2s->Init.AudioFreq = I2S_AUDIOFREQ_44K;
	i2s->Init.CPOL = I2S_CPOL_LOW;
	
	if (HAL_I2S_Init(i2s) != HAL_OK)
	{
		return -RT_ERROR;
	}
	
	__HAL_RCC_DMA2_CLK_ENABLE();
	
	dma->Instance = DMA2_Channel2;
    dma->Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma->Init.PeriphInc = DMA_PINC_DISABLE;
    dma->Init.MemInc = DMA_MINC_ENABLE;
    dma->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma->Init.Mode = DMA_CIRCULAR;
    dma->Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(dma) != HAL_OK)
    {
		return -RT_ERROR;
    }
	
	__HAL_LINKDMA(&(sound->h_i2s), hdmatx, sound->h_dma);
	
	HAL_NVIC_SetPriority(DMA2_Channel2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Channel2_IRQn);
	
	return RT_EOK;
}

static rt_err_t start(struct rt_audio_device *audio, int stream)
{
	struct stm32_sound *sound = RT_NULL;
	RT_ASSERT(audio != RT_NULL);
	sound = (struct stm32_sound *)audio->parent.user_data; (void)sound;
	
	HAL_I2S_Transmit_DMA(&sound->h_i2s, (uint16_t *)sound->tx_fifo, TX_DMA_FIFO_SIZE);
	
	return RT_EOK;
}
 
static rt_err_t stop(struct rt_audio_device *audio, int stream)
{
	struct stm32_sound *sound = RT_NULL;
	RT_ASSERT(audio != RT_NULL);
	sound = (struct stm32_sound *)audio->parent.user_data; (void)sound;
	
	HAL_I2S_DMAStop(&sound->h_i2s);

	return RT_EOK;
}

rt_size_t transmit(struct rt_audio_device *audio, const void *writeBuf, void *readBuf, rt_size_t size)
{
	struct stm32_sound *sound = RT_NULL;
	RT_ASSERT(audio != RT_NULL);
	sound = (struct stm32_sound *)audio->parent.user_data; (void)sound;

	return RT_EOK;
}

static void buffer_info(struct rt_audio_device *audio, struct rt_audio_buf_info *info)
{
	struct stm32_sound *sound = RT_NULL;
	RT_ASSERT(audio != RT_NULL);
	sound = (struct stm32_sound *)audio->parent.user_data; (void)sound;

    /**
     *               TX_FIFO
     * +----------------+----------------+
     * |     block1     |     block2     |
     * +----------------+----------------+
     *  \  block_size  /
     */
    info->buffer      = sound->tx_fifo;
    info->total_size  = TX_DMA_FIFO_SIZE;
    info->block_size  = TX_DMA_FIFO_SIZE / 2;
    info->block_count = 2;
}

static struct rt_audio_ops ops =
{
	.getcaps     = getcaps,
	.configure   = configure,
	.init        = init,
	.start       = start,
	.stop        = stop,
	.transmit    = transmit,
	.buffer_info = buffer_info,
};

static int rt_hw_sound_init(void)
{
	rt_uint8_t *tx_fifo = RT_NULL;
	
	
	rt_hw_spi_device_attach("spi2", "spi20", GPIOB, GPIO_PIN_11);
	stm32_snd.spi_dev = (struct rt_spi_device *) rt_device_find("spi20");
	if (stm32_snd.spi_dev == RT_NULL || stm32_snd.spi_dev->parent.type != RT_Device_Class_SPIDevice) {
		LOG_E("ERROR: SPI device %s not found!", "spi20");
		return -RT_ENOSYS;
	}
	
	/* config spi */
    {
        struct rt_spi_configuration cfg;
        cfg.data_width = 16;
        cfg.mode = RT_SPI_MODE_3 | RT_SPI_MSB; /* SPI Compatible Modes 看例子是3或者2 没有测试 */
        cfg.max_hz = 5 * 1000 * 1000; /* SPI Interface with Clock Speeds 5 MHz */
        rt_spi_configure(stm32_snd.spi_dev, &cfg);
    } /* config spi */
	
	/* 分配 DMA 搬运 buffer */
	tx_fifo = rt_calloc(1, TX_DMA_FIFO_SIZE);
	if(tx_fifo == RT_NULL)
	{
		return -RT_ENOMEM;
	}
    stm32_snd.tx_fifo = tx_fifo;

    /* 注册声卡放音驱动 */
    stm32_snd.device.ops = &ops;
    rt_audio_register(&stm32_snd.device, "sound0", RT_DEVICE_FLAG_WRONLY, &stm32_snd);

	return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_sound_init);

void DMA2_Channel2_IRQHandler(void)
{
	/* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&stm32_snd.h_dma);

    /* leave interrupt */
    rt_interrupt_leave();
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	rt_audio_tx_complete(&stm32_snd.device);
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	rt_audio_tx_complete(&stm32_snd.device);
}


