#搜狗面试题编写

CUDA编程题

题目大意为编写两个数组相加的简单题目

```c++
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <math.h>
#include <time.h>
#include <stdio.h>
#include<cstdlib>

__global__ void addKernel(int *c, const int *a, const int *b，const int n)
{
    unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
  	if(i<n){  //边界条件处理
      	c[i] = a[i] + b[i];
  	} 
}

int main()
{
    const int arraySize = 1024;   //设置数组长度
    //申请主存由于可能面临数组长度过大问题，建议使用堆地址空间
  	int *a=(int *)malloc(arraySize * sizeof(int));
    int *b=(int *)malloc(arraySize * sizeof(int));
    int *c=(int *)malloc(arraySize * sizeof(int));
    //为元素赋值
  	for (int i = 0; i < arraySize; i++){
		h_input[i]==rand()%10; //产生0~9的随机数
	}
    //申请显存空间，默认为global memory
  	int *dev_a = 0;
    int *dev_b = 0;
    int *dev_c = 0;
  	cudaError_t cudaStatus;
  	cudaStatus = cudaMalloc((void**)&dev_a, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }
  	cudaStatus = cudaMalloc((void**)&dev_b, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }
  	cudaStatus = cudaMalloc((void**)&dev_c, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }
  	//数据拷贝
  	cudaStatus = cudaMemcpy(dev_a, a, arraySize * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }
    cudaStatus = cudaMemcpy(dev_b, b, arraySize * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }
  	//分配线程数
  	const size_t block_size = 512;//线程块的大小。有些gpu的线程块最大为512，有些为1024.
  	//分配block数目时，需要考虑是否能除尽的问题
    const size_t num_blocks = (arraySize/block_size) + ((arraySize%block_size) ? 1 : 0);
  	// 调用核函数
    addKernel<<<num_blocks,block_size, size>>>(dev_c, dev_a, dev_b，arraySize);
  	cudaMemcpy(dev_c, c, arraySize * sizeof(int), cudaMemcpyDeviceToHost);
  	//do something about c
  	//....
  
  	Error:
  		free(a);
  		free(b);
  		free(c);
    	cudaFree(dev_c);
    	cudaFree(dev_a);
    	cudaFree(dev_b);
  return 0;
}
	
```



CUDA编写实现对数组求和

```c++
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>

#include <iostream>
using namespace std;

//CUDA语言支持模板操作 使用了模板对数据类型进行了扩展
template<class DType>
__global__ void block_sum(const DType *input,
        DType *per_block_results,
                        const size_t n)
{
    extern __shared__ DType sdata[];  //动态调用的shared memory ，访问了速度shared memory高于global memory
 
    unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
 
    // 一个线程负责把一个元素从全局内存载入到共享内存
    DType x = 0;
    if(i < n){
        x = input[i];
    }
    sdata[threadIdx.x] = x;
    __syncthreads();//等待所有线程把自己负责的元素载入到共享内存
 
    // 块内进行合并操作，每次合并变为一半.注意threadIdx.x是块内的偏移，上面算出的i是全局的偏移。
    for(int offset = blockDim.x / 2;
            offset > 0;
            offset >>= 1)
    {
        if(threadIdx.x < offset)//控制只有某些线程才进行操作。
        {
            sdata[threadIdx.x] += sdata[threadIdx.x + offset];
        }
        __syncthreads();
    }
 
    //每个块的线程0负责存放块内求和的结果
    if(threadIdx.x == 0)
    {
        per_block_results[blockIdx.x] = sdata[0];
    }
}

int main()
{
	const int num_elements=1024;   //设置数组长度
    //分配内存
	float *h_input=(float *)malloc(num_elements*sizeof(float));
	for (int i = 0; i < num_elements; i++){
		h_input[i]=1.0f;
	}
	
    float *d_input = 0;

    cudaMalloc((void**)&d_input, sizeof(float) * num_elements);
    cudaMemcpy(d_input, &h_input, sizeof(float) * num_elements, cudaMemcpyHostToDevice);
 
    const size_t block_size = 512;//线程块的大小。目前有些gpu的线程块最大为512，有些为1024.
    const size_t num_blocks = (num_elements/block_size) + ((num_elements%block_size) ? 1 : 0);
 
    float *d_partial_sums_and_total = 0;//一个线程块一个和，另外加一个元素，存放所有线程块的和。
    cudaMalloc((void**)&d_partial_sums_and_total, sizeof(float) * (num_blocks + 1));
 
    //把每个线程块的和求出来
    block_sum<<<num_blocks,block_size,block_size *sizeof(float)>>>(d_input, d_partial_sums_and_total, num_elements);
 
    
     //再次用一个线程块把上一步的结果求和。
    //注意这里有个限制，上一步线程块的数量，必须不大于一个线程块线程的最大数量，因为这一步得把上一步的结果放在一个线程块操作。
    //即num_blocks不能大于线程块的最大线程数量。
    block_sum<<<1,num_blocks,num_blocks * sizeof(float)>>>(d_partial_sums_and_total, d_partial_sums_and_total + num_blocks, num_blocks);
 
    
	float device_result = 0;
	cudaMemcpy(&device_result, d_partial_sums_and_total + num_blocks, sizeof(float), cudaMemcpyDeviceToHost);
 
    std::cout << "Device sum: " << device_result << std::endl;
 
    // 释放显存容量
  	free(h_input);
    cudaFree(d_input);
    cudaFree(d_partial_sums_and_total);
	
	return 0;
}

```



Linux C编程题

实现getopt

```C++
bool getopt(char **argv,int argc,map<string,string>&mmap,vector<string>& paramter){
    bool flag=true;
    if(argc==1){
        flag=false;
        return flag;
    }
    int index=1;
    bool out=true;
    while(index<argc&&out){
        string key;string value;
        int len=strlen(argv[index]);
        if(len<=1){
            flag=false;
            return flag;
        }
        if(len>2&&argv[index][0]=='-'&&argv[index][1]=='-'){
               key= argv[index][2];

        }
        else if(argv[index][0]=='-'&&argv[index][1]!='-'){
               key+= argv[index][1];
        }
        else{
            out=false;
        }
        index++;
        value+=argv[index][0];
        if(key==""||value=""){
            flag=false;
            return flag;
        }
        else{
             map<string,string>::iterator it;
             it=word.find (key);
             //如果元素不存在，指针指向word.end().
             if(it!=word.end ()){
                 map[key]=value;
                 index++;
             }
             else{
                 flag=false;
                 return flag;
             }
        }
    }
    while(index<argc){
        if(argv[index][0]=='-'){
            flag=false;
            return flag;
        }
        else{
            paramter.insert(string(argv[index][0]));
        }
    }
    return flag;
}

```

