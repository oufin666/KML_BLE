#include "KEY.h"

// 按键状态变量，用于检测按键抬起事件
static uint8_t keyb_last_state = KEY_RELEASED;
static uint8_t keyc_last_state = KEY_RELEASED;
static uint8_t keyb_released_flag = 0;
static uint8_t keyc_released_flag = 0;

/**
 * @brief  按键初始化
 * @retval HAL_StatusTypeDef 初始化状态
 */
HAL_StatusTypeDef KEY_Init(void)
{
    // 初始化按键状态
    keyb_last_state = KEYB_Read();
    keyc_last_state = KEYC_Read();
    keyb_released_flag = 0;
    keyc_released_flag = 0;
    return HAL_OK;
}

/**
 * @brief  读取KEYB按键状态
 * @retval uint8_t 按键状态：KEY_PRESSED(0)或KEY_RELEASED(1)
 */
uint8_t KEYB_Read(void)
{
    return HAL_GPIO_ReadPin(KEYB_PORT, KEYB_PIN);
}

/**
 * @brief  读取KEYC按键状态
 * @retval uint8_t 按键状态：KEY_PRESSED(0)或KEY_RELEASED(1)
 */
uint8_t KEYC_Read(void)
{
    return HAL_GPIO_ReadPin(KEYC_PORT, KEYC_PIN);
}

/**
 * @brief  检查KEYB是否被按下
 * @retval uint8_t 1: 按键被按下，0: 按键未被按下
 */
uint8_t KEYB_IsPressed(void)
{
    return (KEYB_Read() == KEY_PRESSED);
}

/**
 * @brief  检查KEYC是否被按下
 * @retval uint8_t 1: 按键被按下，0: 按键未被按下
 */
uint8_t KEYC_IsPressed(void)
{
    return (KEYC_Read() == KEY_PRESSED);
}

/**
 * @brief  检查KEYB是否处于释放状态
 * @retval uint8_t 1: 按键已释放，0: 按键未释放
 */
uint8_t KEYB_IsReleased(void)
{
    return (KEYB_Read() == KEY_RELEASED);
}

/**
 * @brief  检查KEYC是否处于释放状态
 * @retval uint8_t 1: 按键已释放，0: 按键未释放
 */
uint8_t KEYC_IsReleased(void)
{
    return (KEYC_Read() == KEY_RELEASED);
}

/**
 * @brief  检查KEYB是否刚刚抬起
 * @retval uint8_t 1: 按键刚刚抬起，0: 按键未抬起
 */
uint8_t KEYB_HasReleased(void)
{
    uint8_t result = keyb_released_flag;
    keyb_released_flag = 0; // 清除标志
    return result;
}

/**
 * @brief  检查KEYC是否刚刚抬起
 * @retval uint8_t 1: 按键刚刚抬起，0: 按键未抬起
 */
uint8_t KEYC_HasReleased(void)
{
    uint8_t result = keyc_released_flag;
    keyc_released_flag = 0; // 清除标志
    return result;
}

/**
 * @brief  更新按键状态，检测按键抬起事件
 * @retval 无
 */
void KEY_Update(void)
{
    uint8_t keyb_current = KEYB_Read();
    uint8_t keyc_current = KEYC_Read();
    
    // 检测KEYB抬起事件
    if (keyb_last_state == KEY_PRESSED && keyb_current == KEY_RELEASED)
    {
        keyb_released_flag = 1;
    }
    
    // 检测KEYC抬起事件
    if (keyc_last_state == KEY_PRESSED && keyc_current == KEY_RELEASED)
    {
        keyc_released_flag = 1;
    }
    
    // 更新上一次状态
    keyb_last_state = keyb_current;
    keyc_last_state = keyc_current;
}