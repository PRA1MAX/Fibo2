import streamlit as st
import pandas as pd
import os
import altair as alt

def process_dat_file(file_path):
    """Read and process a single .dat file"""
    column_names = ['Fibonacci index', 'Time (s)', 'Size (bytes)']
    
    df = pd.read_csv(
        file_path,
        delimiter='|',
        comment='#',
        skipinitialspace=True,
        header=None,
        names=column_names
    )
    
    df['Time (s)'] = df['Time (s)'].str.replace('s', '').astype(float)
    df['Size (bytes)'] = df['Size (bytes)'].str.replace('B', '').astype(int)
    return df

def main():
    st.title('Fibonacci V2')
    
    data_folder = 'data'
    dat_files = [f for f in os.listdir(data_folder) if f.endswith('.dat')]
    
    if not dat_files:
        st.error("No .dat files found!")
        return

    # Process all files and combine into a single DataFrame
    all_data = []
    for file in dat_files:
        file_path = os.path.join(data_folder, file)
        df = process_dat_file(file_path)
        method = os.path.splitext(file)[0]
        df['Method'] = method  # Add method column for coloring
        all_data.append(df)
    
    combined_df = pd.concat(all_data, ignore_index=True)

    # Sidebar for interactivity
    st.sidebar.header("")
    
    # Allow users to select methods to display
    methods = combined_df['Method'].unique()
    selected_methods = st.sidebar.multiselect(
        "",
        options=methods,
        default=methods  # Show all methods by default
    )
    
    # Filter data based on selected methods
    filtered_df = combined_df[combined_df['Method'].isin(selected_methods)]
    
    # Create time comparison chart with smaller points
    time_chart = alt.Chart(filtered_df).mark_line(
        point=alt.OverlayMarkDef(shape='circle', size=2)  # Reduced size from 30 to 10
    ).encode(
        x=alt.X('Fibonacci index:Q', title='Fibonacci Index'),
        y=alt.Y('Time (s):Q', title='Time (s)'),  # Removed logarithmic scale
        color='Method:N',
        tooltip=['Method', 'Fibonacci index', 'Time (s)']
    ).properties(
        title='',
        width=800,
        height=300
    ).interactive()  # Enable zoom and pan
    
    st.altair_chart(time_chart)


if __name__ == '__main__':
    main()