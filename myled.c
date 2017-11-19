/* myled.c
 * @author Mounika Reddy Edu;a
 * @date   11 November 2017
 * @brief  A kernel module for controlling on board LED parameters like
 * period/rate of flashing,dutycycle of flashing,state of led
 * The sysfs entry appears at /sys/ebb/led53
 *
 * Credits: This homework is done with refernce provided in the HW
 * Reference  - @see http://www.derekmolloy.ie/
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>      
#include <linux/kobject.h>    
#include <linux/kthread.h>    
#include <linux/delay.h>      
#include <linux/timer.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mounika Reddy Edula");
MODULE_DESCRIPTION("Linux LED driver parameters blink period(2ms,100s) dutycycle(10,100) led_state(ledon,ledoff,flash)");
MODULE_VERSION("0.1");

static unsigned int gpioLED = 53;           // User led- 1 53
module_param(gpioLED, uint, S_IRUGO);       // Param desc. S_IRUGO can be read/not changed
MODULE_PARM_DESC(gpioLED, " GPIO LED number (default=53)");  

//default values
static unsigned int blinkPeriod = 1000;     // The blink period in 1s
static unsigned int blinkDutyCycle = 50;    //duty cycle 50%
module_param(blinkPeriod, uint, S_IRUGO);   // Param desc. S_IRUGO can be read/not changed
MODULE_PARM_DESC(blinkPeriod, " LED blink period in ms (min=1, default=1000, max=100000)");

static char ledName[7] = "ledXXX";          
static bool ledOn = 0;                     //ledOn for storing the state of the led
enum led_state_t { LEDON,LEDOFF,FLASH };   //modes of led
static volatile enum led_state_t led_state = LEDON;  //default mode
static struct timer_list my_timer; //timer for perodic call back

/* the attribute_show is callback for if the file is read in user space*/
static ssize_t state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
   switch(ledOn){
      case 0:   return sprintf(buf, "ledoff\n");       // Display the state of led
      case 1:    return sprintf(buf, "ledon\n");
      default:    return sprintf(buf, "LKM Error\n"); // Cannot get here
   }
}

/* the attribute_store is callback for if the file is written in user space*/
static ssize_t state_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
   // the count-1 is important as otherwise the \n is used in the comparison
   if (strncmp(buf,"ledoff",count-1)==0) { led_state = LEDOFF; }   // strncmp() compare with fixed number chars
   else if (strncmp(buf,"ledon",count-1)==0) {led_state = LEDON; }
   else if (strncmp(buf,"flash",count-1)==0) { led_state = FLASH; }
   return count;
}

/* the attribute_show is callback for if the file is read in user space*/
static ssize_t period_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
   return sprintf(buf, "%d\n", blinkPeriod);
}

/* the attribute_store is callback for if the file is written in user space*/
static ssize_t period_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
   unsigned int period;                     
   sscanf(buf, "%du", &period);             
   if ((period>1)&&(period<=100000)){        // Must be 2ms or greater, 100secs or less
      blinkPeriod = period;                 
   }
   return period;
}

/* the attribute_show is callback for if the file is read in user space*/
static ssize_t dutycycle_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
   return sprintf(buf, "%d\n", blinkDutyCycle);
}

/* the attribute_store is callback for if the file is written in user space*/
static ssize_t dutycycle_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
   unsigned int dutycycle;                     
   sscanf(buf, "%du", &dutycycle);             
   if ((dutycycle>10)&&(dutycycle<=100)){        // Must be 10% or greater, 100% or less
       blinkDutyCycle = dutycycle;                 
   }
   return dutycycle;
}

/*
 *  Attributes with 0644 so that only root user is given the permissions to set the parameters
 */
static struct kobj_attribute period_attr = __ATTR(blinkPeriod, 0644, period_show, period_store);
static struct kobj_attribute dutycycle_attr = __ATTR(blinkDutyCycle, 0644, dutycycle_show, dutycycle_store);
static struct kobj_attribute state_attr = __ATTR(led_state,0644,state_show,state_store);


static struct attribute *led_attrs[] = {
   &period_attr.attr,                       // The period at which the LED flashes
   &dutycycle_attr.attr,                    //dutycycle
   &state_attr.attr,                        //led state attribute
   NULL,
};


static struct attribute_group attr_group = {
   .name  = ledName,                        // The name is generated in ebbLED_init()
   .attrs = led_attrs,                      // The attributes array defined just above
};

static struct kobject *led_kobj;            /// The pointer to the kobject

/**timer callback for period*/
void my_timer_callback(unsigned long data)
{
   int ret;
   printk("timer expired\n");
   if(led_state == FLASH) ledOn = !ledOn;
   if(led_state == LEDON) ledOn = true;
   if(led_state == LEDOFF) ledOn = false;
   gpio_set_value(gpioLED,ledOn); 
   ret = mod_timer(&my_timer, jiffies+msecs_to_jiffies((blinkPeriod*blinkDutyCycle)/100));
   if(ret)
   printk("Error in mod timer\n");
}

/* @brief The LKM initialization function*/
static int __init LED_init(void){
   int result = 0;
   int ret;
   printk(KERN_INFO "LED: Initializing the LED driver\n");
   sprintf(ledName, "led%d", gpioLED);      // Create the gpio115 name for /sys/ebb/led53

   led_kobj = kobject_create_and_add("ecen5013", kernel_kobj->parent); // kernel_kobj points to /sys/kernel
   if(!led_kobj){
      printk(KERN_ALERT "LED: failed to create kernel object\n");
      return -ENOMEM;
   }
   // add the attributes to /sys/ecen5013/ -- for example, /sys/ebb/led53/ledOn
   result = sysfs_create_group(led_kobj, &attr_group);
   if(result) {
      printk(KERN_ALERT "LED: failed to create sysfs group\n");
      kobject_put(led_kobj);                // clean up -- remove the kobject sysfs entry
      return result;
   }
   ledOn = true;
   ret = gpio_request(gpioLED, "sysfs");   
   if(ret)
   printk("Gpio request failed\n");
   ret = gpio_direction_output(gpioLED, ledOn); 
   if(ret)
   printk("Setting gpio direction failed\n");
   ret = gpio_export(gpioLED, false);  
   if(ret)
   printk("exporting gpio failed\n");

   setup_timer(&my_timer,my_timer_callback,0);
   ret = mod_timer(&my_timer, jiffies+msecs_to_jiffies((blinkPeriod*blinkDutyCycle)/100));
   if(ret)
   printk("Error in mod timer\n");
   return result;
}

/** @brief The module cleanup function*/
static void __exit LED_exit(void){
   int ret;
   kobject_put(led_kobj);                  
   gpio_set_value(gpioLED, 0);              
   gpio_unexport(gpioLED);                  
   gpio_free(gpioLED);                    
   ret = del_timer(&my_timer);
   if(ret)
   {
      printk("Timer is still in use\n");
   }
   printk(KERN_INFO "LED: Goodbye from the LED Driver!\n");
}

//To indicate the entry and exit functions in the module
module_init(LED_init);
module_exit(LED_exit);
