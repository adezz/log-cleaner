# log-cleaner
日志清理工具  
作者：土司 skyer  
 
优点：  

1、保存了修改前的相关时间属性，以免被人发觉文件修改  
2、算法参考：https://www.linuxquestions.org/questions/programming-9/fastest-memmem-4175481626/


缺点：  

1、建议64位s系统下进行使用，MapViewOfFile在32位只能映射4G内存空间  
2、只处理*.txt,\*.log文件  

学习：  

1、这里对文件的内存操作是通过映射内存实现的，然后再用算法来进行遍历该内存，文件递归，内容通过偏移循环遍历修改，这也是一种好方法！
