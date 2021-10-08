"Onyx"

[Here we have a little sample story based on Slavic folk tales. To try out this story, click the Go! button above. Then type TEST ME and press return to try out the actions of the story.]

The Deep Forest is a room. "You are lost in a deep forest. The trees grow so close together that you cannot see the sun or the sky, and no grass or small plants thrive on the ground."

[Try changing the words inside the quote marks to give the room a new description.]


Baba Yaga's Garden is west of the Deep Forest. "In this garden grow plants that eat insects and plants that eat birds. It is bordered by a bone fence, and the pavements are made of a giant's kneecaps. 

In the center of the garden is Baba Yaga's hut, which walks on chicken legs. Currently the hut is squatting on the ground."

[You can add more plants to Baba's garden by writing something like 'A carnivorous plant is in Baba Yaga's Garden.']


Baba Yaga's Hut is inside from Baba Yaga's Garden. "From in here, there is no evidence of the hut's chicken nature except that the wallpaper is made of yellow-white feathers." 

A gigantic mortar is in Baba Yaga's Hut. "One corner of the hut is taken up by the gigantic mortar, big enough to stand in." It is an enterable vehicle. The description of the gigantic mortar is "Baba Yaga flies around inside it whenever she is in too much of a hurry to make her hut walk."

Baba Yaga is a woman in Baba Yaga's Hut. "Baba Yaga sits at her work table, sewing symbols into a magic towel." The description of Baba Yaga is "Her teeth are made of iron and her eyes can see in the dark. Though she is old, her fingers are nimble, and she has forgotten nothing that she has ever seen."

Instead of entering the mortar in the presence of Baba Yaga:
	say "'Oh, I wouldn't do that, Foolish Ivan,' says Baba Yaga. 'Where it would take you, you're not ready to go.'"

Baba Yaga is carrying a magic towel. The description of the magic towel is "To judge by the spell worked into its fringes, this is a towel that can turn into a bridge across any river or chasm."

The description of the player is "Some people call you Ivan the Fool, but you pay no attention to their rudeness."

The player is carrying a magic ball. The description of the magic ball is "Whenever you drop this, it rolls in the way you should go."  

After dropping the ball:
	if an unvisited room (called target location) is adjacent to the location:
		let way be the best route from the location to the target location;
		say "It rolls suggestively towards [the way]. You pick it up again before it can get too far ahead of you.";
	else:
		say "The ball rolls around uncertainly and then comes back to sit at your feet with such an apologetic air that you have to pick it back up.";
	now the player carries the ball.

Test me with "look / take inventory / examine the ball / drop the ball / west / examine me / in / look at baba / examine towel / drop ball / look at mortar / get in mortar".