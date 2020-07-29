#pragma once
#include <vector>
#include <type_traits>
#include <algorithm>
#include <stdexcept>
#include <core/platform/platform.hpp>
#include <core/types/primitives.hpp>

/**
 * @file sparse_set.hpp
 */


namespace args::core
{
	/**@class sparse_set
	 * @brief Quick lookup contiguous set. A sparse set uses a dense and a sparse array to allow quick lookup of it's values whilst maintaining contiguous iteration.
	 * @note Inserting items with large value differences does explode the sparse array and thus increases memory size.
	 *       If you need support for items with large value differences such as hashes then please use the sparse_map.
	 * @ref args::core::sparse_map
	 * @tparam atomic_type The type to be used as the value (must be unsigned).
	 * @tparam dense_type Container to be used to store the values.
	 * @tparam sparse_type Container to be used to store the index mapping into dense container.
	 * @note With default container parameters iterators may be invalidated upon resize. See reference of std::vector.
	 * @note Removing item might invalidate the iterator of the last item in the dense container.
	 */
	template <typename value_type, template<typename...> typename dense_type = std::vector, template<typename...> typename sparse_type = std::vector>
	class sparse_set
	{
		static_assert(std::is_unsigned_v<value_type>, "atomic_type must an unsigned type.");
	public:
		using sparse_container = sparse_type<value_type>;
		using dense_container = dense_type<value_type>;

		using reference = value_type&;
		using const_reference = const value_type&;

		using iterator = typename dense_container::iterator;
		using const_iterator = typename dense_container::const_iterator;

	private:
		dense_container m_dense;
		sparse_container m_sparse;

		size_type m_size = 0;
		size_type m_capacity = 0;

	public:
		A_NODISCARD iterator begin() { return m_dense.begin(); }
		A_NODISCARD const_iterator begin() const { return m_dense.cbegin(); }

		A_NODISCARD iterator end() { return m_dense.begin() + m_size; }
		A_NODISCARD const_iterator end() const { return m_dense.cbegin() + m_size; }

		/**@brief Returns the amount of items in the sparse_set.
		 * @returns size_type Current amount of items contained in sparse_set.
		 */
		A_NODISCARD size_type size() const noexcept { return m_size; }

		/**@brief Returns the capacity of items the sparse_set could at least store without invalidating the iterators.
		 * @returns size_type Current capacity of the dense container.
		 */
		A_NODISCARD size_type capacity() const noexcept { return m_capacity; }

		/**@brief Returns whether the sparse_set is empty.
		 * @returns bool True if the sparse_set is empty, otherwise false.
		 */
		A_NODISCARD bool empty() const noexcept { return m_size == 0; }

		/**@brief Clears sparse_set.
		 * @note Will not update capacity.
		 */
		void clear() noexcept { m_size = 0; }

		/**@brief Reserves space in sparse_set for more items.
		 * @param size Amount of items to reserve space for (would be the new capacity).
		 * @note Will update capacity if resize happened.
		 */
		void reserve(size_type size)
		{
			if (size > m_capacity)
			{
				m_dense.resize(size, 0);
				m_sparse.resize(size, 0);
				m_capacity = size;
			}
		}

#pragma region count
		/**@brief Returns the amount of items of a certain value.
		 * @param val Value to look for.
		 * @returns size_type Amount of items found (either 0 or 1).
		 * @note Function is only available for compatibility reasons, it is advised to use contains instead.
		 * @ref args::core::sparse_set::contains
		 */
		A_NODISCARD size_type count(const_reference val) const
		{
			return contains(val);
		}

		/**@brief Returns the amount of items of a certain value.
		 * @param val Value to look for.
		 * @returns size_type Amount of items found (either 0 or 1).
		 * @note Function is only available for compatibility reasons, it is advised to use contains instead.
		 * @ref args::core::sparse_set::contains
		 */
		A_NODISCARD size_type count(value_type&& val) const
		{
			return contains(val);
		}
#pragma endregion

#pragma region contains
		/**@brief Checks whether a certain value is contained in the sparse_set.
		 * @param val Value to check for.
		 * @returns bool True if the value was found, otherwise false.
		 */
		A_NODISCARD bool contains(const_reference val) const
		{
			return val < m_capacity&&
				m_sparse[val] < m_size&&
				m_dense[m_sparse[val]] == val;
		}

		/**@brief Checks whether a certain value is contained in the sparse_set.
		 * @param val Value to check for.
		 * @returns bool True if the value was found, otherwise false.
		 */
		A_NODISCARD bool contains(value_type&& val) const
		{
			return val < m_capacity&&
				m_sparse[val] < m_size&&
				m_dense[m_sparse[val]] == val;
		}

		/**@brief Checks if all items in sparse_set are inside this set as well.
		 * @param other Other sparse_set to check against.
		 * @returns bool True if all items in other are also in this sparse_set, otherwise false.
		 */
		A_NODISCARD bool contains(const sparse_set<value_type>& other) const
		{
			if (other.m_size == 0)
				return true;

			if (m_size == 0 || m_size < other.m_size)
				return false;

			bool overlap = true;
			for (const_reference item : other)
				overlap = overlap && contains(item);

			return overlap;
		}
#pragma endregion

		/**@brief Checks if all items are the same for both sparse_sets.
		 * @param other Other sparse_set to check against.
		 * @returns bool True if both sets are the same size and contain the same items, otherwise false.
		 */
		A_NODISCARD bool equals(const sparse_set<value_type>& other) const
		{
			if (m_size == other.m_size)
			{
				bool equal = true;
				for (int i = 0; i < m_size; i++)
					equal = equal && other.contains(m_dense[i]);

				return equal;
			}

			return false;
		}

		/**@brief Checks if all items are the same for both sparse_sets.
		 * @param other Other sparse_set to check against.
		 * @returns bool True if both sets are the same size and contain the same items, otherwise false.
		 */
		A_NODISCARD bool operator==(const sparse_set<value_type>& other) const
		{
			if (m_size == other.m_size)
			{
				bool equal = true;
				for (int i = 0; i < m_size; i++)
					equal = equal && other.contains(m_dense[i]);

				return equal;
			}

			return false;
		}

#pragma region find
		/**@brief Finds the iterator of a value.
		 * @param val Value to find.
		 * @returns Iterator to the value if found, otherwise end.
		 */
		A_NODISCARD iterator find(const_reference val)
		{
			if (contains(val))
				return begin() + m_sparse[val];
			return end();
		}

		/**@brief Finds the iterator of a value.
		 * @param val Value to find.
		 * @returns Iterator to the value if found, otherwise end.
		 */
		A_NODISCARD const_iterator find(const_reference val) const
		{
			if (contains(val))
				return begin() + m_sparse[val];
			return end();
		}
#pragma endregion

#pragma region insert
		/**@brief Inserts new item into sparse_set
		 * @param val Value to insert.
		 * @returns std::pair<iterator, bool> Iterator at the location of the item and true if succeeded, end and false if it didn't succeed.
		 */
		std::pair<iterator, bool> insert(const_reference val)
		{
			if (!contains(val))
			{
				if (val >= m_capacity)
					reserve(val + 1);

				auto itr = m_dense.begin() + m_size;
				*itr = val;
				m_sparse[val] = (value_type)m_size;
				++m_size;
				return std::make_pair(itr, true);
			}
			return std::make_pair(m_dense.end(), false);
		}

		/**@brief Inserts new item into sparse_set
		 * @param val Value to insert.
		 * @returns std::pair<iterator, bool> Iterator at the location of the item and true if succeeded, end and false if it didn't succeed.
		 */
		std::pair<iterator, bool> insert(value_type&& val)
		{
			if (!contains(val))
			{
				if (val >= m_capacity)
					reserve(val + 1);

				auto itr = m_dense.begin() + m_size;
				*itr = val;
				m_sparse[val] = (value_type)m_size;
				++m_size;
				return std::make_pair(itr, true);
			}
			return std::make_pair(m_dense.end(), false);
		}
#pragma endregion

#pragma region operator[]
		/**@brief Returns item from dense container.
		 * @param index Index of item in dense container.
		 */
		A_NODISCARD reference operator[](size_type&& index)
		{
			if (index < 0 || index > m_size)
				throw std::out_of_range("Index out of range.");
			return m_dense[index];
		}

		/**@brief Returns item from dense container.
		 * @param index Index of item in dense container.
		 */
		A_NODISCARD reference operator[](const size_type& index)
		{
			return m_dense[index];
		}

		/**@brief Returns item from dense container.
		 * @param index Index of item in dense container.
		 */
		A_NODISCARD const_reference operator[](size_type&& index) const
		{
			return m_dense[index];
		}

		/**@brief Returns item from dense container.
		 * @param index Index of item in dense container.
		 */
		A_NODISCARD const_reference operator[](const size_type& index) const
		{
			return m_dense[index];
		}
#pragma endregion

		/**@brief Erases item from sparse_set.
		 * @param val Value that needs to be erased.
		 */
		size_type erase(const_reference val)
		{
			if (contains(val))
			{
				m_dense[m_sparse[val]] = m_dense[m_size - 1];
				m_sparse[m_dense[m_size - 1]] = m_sparse[val];
				--m_size;
				return true;
			}
			return false;
		}
	};
}