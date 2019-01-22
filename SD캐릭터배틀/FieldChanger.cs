using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FieldChanger : MonoBehaviour {

    // Use this for initialization
    void Start()
    {
        ChangeField();
    }
	
	// Update is called once per frame
	void Update () {
		
	}

    public void ChangeField()
    {
        int r = Random.Range(0, 5);
        GetComponent<SpriteRenderer>().sprite = Resources.Load<Sprite>("Image/Field" + r);
    }
}
