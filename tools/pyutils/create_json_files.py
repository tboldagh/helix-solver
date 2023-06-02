import json

print(123)
input_file = '../../../config.json'
output_prefix = 'output'
value_key = 'event_skip'
substitute_values = [0, 1, 2]

def create_json_files(input_file, output_prefix, value_key, substitute_values):
    with open(input_file, 'r') as f:
        data = json.load(f)
        array = data[value_key]

        print("file opened")
        for i, substitute_value in enumerate(substitute_values):
            data[value_key] = [substitute_value] * len(array)
            output_file = f"{output_prefix}_{i}.json"
            print(output_file)
            with open(output_file, 'w') as outfile:
                json.dump(data, outfile, indent=4)
            print(f"Created {output_file}")

# Example usage


#create_json_files(input_file, output_prefix, value_key, substitute_values)