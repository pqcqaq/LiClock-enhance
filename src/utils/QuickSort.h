#ifndef QUICK_SORT_H
#define QUICK_SORT_H

// 通用快速排序模板
template <typename T, typename Compare>
void quickSort(T arr[], int left, int right, Compare less)
{
    if (left >= right)
        return;

    int i = left;
    int j = right;
    T pivot = arr[left];

    while (i < j)
    {
        while (i < j && !less(arr[j], pivot))
            j--;
        if (i < j)
            arr[i++] = arr[j];

        while (i < j && less(arr[i], pivot))
            i++;
        if (i < j)
            arr[j--] = arr[i];
    }

    arr[i] = pivot;

    quickSort(arr, left, i - 1, less);
    quickSort(arr, i + 1, right, less);
}

#endif // QUICK_SORT_H