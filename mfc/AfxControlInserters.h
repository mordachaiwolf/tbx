#pragma once

#include <iterator>

namespace Toolbox {

	// insert iterator for List or Combo box controls
	template <typename C, typename T1, typename T2>
	class ControlBoxInsertIterator // : public std::iterator<std::output_iterator_tag, std::pair<T1, T2>>
	{
	public:
		ControlBoxInsertIterator(C & control, int index = -1) :
			m_control(control), m_index(index)
		{
		}

		void operator = (std::pair<T1, T2> item)
		{
			m_control.SetItemData(m_control.InsertString(m_index, item.first), (DWORD)item.second);
		}

	private:
		C &		m_control;
		int		m_index;
	};

	// partial-specialization of an insert iterator for List or Combo box where you want to insert strings only (no data)
	template <typename C>
	class ControlBoxInsertIterator<C, const TCHAR *, void> // : public std::iterator<std::output_iterator_tag, const TCHAR *>
	{
		using ThisControlBoxInsertIterator = ControlBoxInsertIterator<C, const TCHAR *, void>;

	public:
		ControlBoxInsertIterator(C & control, int index = -1) :
			m_control(control), m_index(index)
		{
		}

		void operator = (const TCHAR * pszNextItem)
		{
			m_control.InsertString(m_index, pszNextItem);
		}

		ThisControlBoxInsertIterator & operator*() { return *this; }
		ThisControlBoxInsertIterator & operator++() { return *this; }
		ThisControlBoxInsertIterator & operator++(int) { return *this; }

	private:
		C &		m_control;
		int		m_index;
	};

	template <typename T1, typename T2>
	class ListBoxInsertIterator : public ControlBoxInsertIterator<CListBox, T1, T2>
	{
	public:
		ListBoxInsertIterator(CListBox & control, int index = -1) : ControlBoxInsertIterator<CListBox, T1, T2>(control, index) { }
	};

	template <typename T1, typename T2>
	class ComboBoxInsertIterator : public ControlBoxInsertIterator<CComboBox, T1, T2>
	{
	public:
		ComboBoxInsertIterator(CComboBox & control, int index = -1) : ControlBoxInsertIterator<CComboBox, T1, T2>(control, index) { }
	};

	//////////////////////////////////////////////////////////////////////////
	// Label Only Insert Iterators

	template <typename C>
	class ControlBoxLabelInsertIterator // : public std::iterator<std::output_iterator_tag, const TCHAR *>
	{
	public:
		ControlBoxLabelInsertIterator(C & control, int index = -1) :
			m_control(control), m_index(index)
		{
		}

		void operator = (const TCHAR * item)
		{
			m_control.InsertString(m_index, item);
		}

		ControlBoxLabelInsertIterator & operator*() { return *this; }
		ControlBoxLabelInsertIterator & operator++() { return *this; }
		ControlBoxLabelInsertIterator & operator++(int) { return *this; }

	private:
		C &		m_control;
		int		m_index;
	};

	template <typename T>
	class ListBoxLabelInsertIterator : public ControlBoxLabelInsertIterator<CListBox>
	{
	public:
		ListBoxLabelInsertIterator(CListBox & control, int index = -1) : ControlBoxLabelInsertIterator<CListBox>(control, index) { }
	};

	template <typename T>
	class ComboBoxLabelInsertIterator : public ControlBoxLabelInsertIterator<CComboBox>
	{
	public:
		ComboBoxLabelInsertIterator(CComboBox & control, int index = -1) : ControlBoxLabelInsertIterator<CComboBox>(control, index) { }
	};

}